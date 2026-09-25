#pragma once

#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWindow>
#include <QTest>
#include <QtQuickTest/quicktest.h>

#include "wisdom-chess/engine/board.hpp"
#include "wisdom-chess/ui/qml/main/game_model.hpp"
#include "wisdom-chess/ui/qml/main/pieces_model.hpp"
#include "wisdom-chess/ui/qml/main/ui_types.hpp"

namespace wisdom::ui::test
{
    // Lets the tests read the board the model holds.
    class InspectableGameModel : public GameModel
    {
    public:
        [[nodiscard]] auto
        board() const
            -> const wisdom::Board&
        {
            return getGame()->getBoard();
        }

        // Mark the side to move as computer-controlled without starting a
        // search, so tests can exercise input during the engine's turn.
        void makeCurrentPlayerComputer()
        {
            auto game = getGame();
            auto who = game->getCurrentTurn();

            if (who == wisdom::Color::White)
                game->setWhitePlayer (wisdom::Player::ChessEngine);
            else
                game->setBlackPlayer (wisdom::Player::ChessEngine);
        }

        using GameModel::isHoldingAMove;
    };

    // The application as main.cpp assembles it: both models, the signal
    // connections between them and the real QML, loaded from resources.
    class Application
    {
    public:
        explicit Application (const char* main_qml_file = "main/desktop_main.qml")
        {
            QObject::connect (&game_model, &GameModel::engineMoved,
                              &pieces_model, &PiecesModel::playerMoved);
            QObject::connect (&game_model, &GameModel::humanMoved,
                              &pieces_model, &PiecesModel::playerMoved);
            QObject::connect (&game_model, &GameModel::gameStarted,
                              &pieces_model, &PiecesModel::newGame);

            QObject::connect (&my_engine, &QQmlEngine::warnings, &my_engine,
                [this] (const QList<QQmlError>& errors)
                {
                    for (const auto& error : errors)
                        warnings << error.toString();
                });

            auto* context = my_engine.rootContext();
            context->setContextProperty (QStringLiteral ("_myGameModel"), &game_model);
            context->setContextProperty (QStringLiteral ("_myPiecesModel"), &pieces_model);

            my_engine.load (QUrl {
                QStringLiteral ("qrc:/qt/qml/WisdomChess/") + QString::fromLatin1 (main_qml_file)
            });
            game_model.start();
        }

        // A test can end with a menu or dialog open, which the application
        // never has when its window goes away. Closing them first keeps
        // them from outliving the window; on macOS the test after one left
        // open crashed.
        ~Application()
        {
            if (window() == nullptr)
                return;

            auto open_popups = [this]
            {
                QList<QObject*> result;
                for (auto* object : window()->findChildren<QObject*>())
                {
                    if (object->inherits ("QQuickPopup") && object->property ("visible").toBool())
                        result << object;
                }
                return result;
            };

            for (auto* popup : open_popups())
                QMetaObject::invokeMethod (popup, "close");

            if (!QTest::qWaitFor ([&] { return open_popups().isEmpty(); }))
                qWarning ("A menu or dialog was still open when the application was destroyed");
        }

        Application (const Application&) = delete;
        auto operator= (const Application&) -> Application& = delete;

        [[nodiscard]] auto
        window() const
            -> QQuickWindow*
        {
            auto roots = my_engine.rootObjects();
            return roots.isEmpty() ? nullptr : qobject_cast<QQuickWindow*> (roots.first());
        }

        // Change one game setting the way the settings dialog's Apply does.
        template <typename Value>
        void changeGameSetting (const char* name, Value value)
        {
            auto settings = game_model.cloneGameSettings();
            const auto& meta_object = GameSettings::staticMetaObject;
            auto property = meta_object.property (meta_object.indexOfProperty (name));

            property.writeOnGadget (&settings, QVariant::fromValue (value));
            game_model.setGameSettings (settings);
        }

        // Without this the engine starts thinking after White's first move.
        void makeBothPlayersHuman()
        {
            changeGameSetting ("blackPlayer", wisdom::ui::Player::Human);
        }

        [[nodiscard]] auto
        engine()
            -> QQmlApplicationEngine&
        {
            return my_engine;
        }

        // A button, menu item, radio button or check box that can be seen,
        // by its label. Popups show their items in the window's overlay,
        // which is part of the same item tree.
        [[nodiscard]] auto
        buttonWithText (const QString& text) const
            -> QQuickItem*
        {
            for (auto* item : itemsWithProperties ("text", "checkable"))
            {
                if (item->property ("text").toString() == text && isShown (item))
                    return item;
            }
            return nullptr;
        }

        [[nodiscard]] auto
        shownItemsOfClass (const char* class_name) const
            -> QList<QQuickItem*>
        {
            QList<QQuickItem*> result;
            for (auto* item : itemsWithProperties ("visible", "enabled"))
            {
                if (item->inherits (class_name) && isShown (item))
                    result << item;
            }
            return result;
        }

        // A Dialog or Menu declared in the QML, by its title. Popups are
        // objects, not items, and belong to the object that declares them.
        [[nodiscard]] auto
        popupWithTitle (const QString& title) const
            -> QObject*
        {
            for (auto* object : window()->findChildren<QObject*>())
            {
                if (object->inherits ("QQuickPopup")
                    && object->property ("title").toString() == title)
                {
                    return object;
                }
            }
            return nullptr;
        }

        // Whether any shown Text item holds this text. Rich text is
        // compared as written in the QML.
        [[nodiscard]] auto
        showsText (const QString& text) const
            -> bool
        {
            for (auto* item : itemsWithProperties ("text", "wrapMode"))
            {
                if (item->property ("text").toString().contains (text) && isTextShown (item))
                    return true;
            }
            return false;
        }

        // Whether the dialog lays out this text in a box big enough to hold
        // it, inside the dialog's content area and above its buttons.
        [[nodiscard]] auto
        dialogFitsText (const QObject* dialog, const QString& text) const
            -> bool
        {
            if (!QQuickTest::qWaitForPolish (window()))
                return false;

            auto* content = dialog->property ("contentItem").value<QQuickItem*>();
            auto* footer = dialog->property ("footer").value<QQuickItem*>();
            if (content == nullptr || footer == nullptr)
                return false;

            auto content_rect = sceneRect (content);
            auto footer_top = footer->mapToScene (QPointF { 0, 0 }).y();
            for (auto* item : itemsWithProperties ("text", "wrapMode"))
            {
                if (!item->property ("text").toString().contains (text) || !isTextShown (item))
                    continue;

                return item->height() >= item->property ("paintedHeight").toDouble()
                    && item->width() >= item->property ("paintedWidth").toDouble()
                    && content_rect.contains (sceneRect (item))
                    && content_rect.bottom() <= footer_top;
            }
            return false;
        }

        // Clicks the middle of the item as it will be drawn. A person only
        // ever clicks what has been drawn, and Qt lays items out just before
        // drawing them. Until then an item's position can be stale: a
        // dialog's buttons all start at the same place, so without the wait
        // a click meant for No lands on Yes (Qt 6.9).
        void clickItem (QQuickItem* item)
        {
            if (!QQuickTest::qWaitForPolish (window()))
                QFAIL( "The window was not laid out in time" );

            auto center = item->mapToScene (QPointF { item->width() / 2, item->height() / 2 });
            QTest::mouseClick (window(), Qt::LeftButton, Qt::NoModifier, center.toPoint());
            QCoreApplication::processEvents();
        }

        // Every delegate stands on a square that holds a piece, and there
        // are as many delegates as pieces.
        [[nodiscard]] auto
        piecesMatchTheBoard() const
            -> bool
        {
            int on_board = 0;
            for (int row = 0; row < wisdom::Num_Rows; row++)
            {
                for (int column = 0; column < wisdom::Num_Columns; column++)
                {
                    if (game_model.board().pieceAt (row, column) != wisdom::Piece_And_Color_None)
                        on_board++;
                }
            }

            auto delegates = pieces();
            if (delegates.size() != on_board)
                return false;

            for (auto* piece : delegates)
            {
                auto row = piece->property ("row").toInt();
                auto column = piece->property ("column").toInt();
                if (game_model.board().pieceAt (row, column) == wisdom::Piece_And_Color_None)
                    return false;
            }
            return true;
        }

        [[nodiscard]] auto
        squareSize() const
            -> double
        {
            return window()->property ("squareSize").toDouble();
        }

        [[nodiscard]] auto
        squares() const
            -> QList<QQuickItem*>
        {
            return itemsWithProperties ("boardRow", "bgColor");
        }

        [[nodiscard]] auto
        pieces() const
            -> QList<QQuickItem*>
        {
            return itemsWithProperties ("isCastlingRook", "castlingSourceColumn");
        }

        [[nodiscard]] auto
        squareAt (const char* coord_text) const
            -> QQuickItem*
        {
            auto coord = wisdom::coordParse (coord_text);
            for (auto* square : squares())
            {
                if (square->property ("boardRow").toInt() == coord.row<int>()
                    && square->property ("boardColumn").toInt() == coord.column<int>())
                {
                    return square;
                }
            }
            return nullptr;
        }

        [[nodiscard]] auto
        pieceAt (const char* coord_text) const
            -> QQuickItem*
        {
            auto coord = wisdom::coordParse (coord_text);
            for (auto* piece : pieces())
            {
                if (piece->property ("row").toInt() == coord.row<int>()
                    && piece->property ("column").toInt() == coord.column<int>())
                {
                    return piece;
                }
            }
            return nullptr;
        }

        void click (const char* coord_text)
        {
            clickItem (squareAt (coord_text));
        }

        // A move the way a player makes one: click the piece, click the target.
        void move (const char* src_text, const char* dst_text)
        {
            click (src_text);
            click (dst_text);
        }

        [[nodiscard]] auto
        boardPieceAt (const char* coord_text) const
            -> ColoredPiece
        {
            return game_model.board().pieceAt (wisdom::coordParse (coord_text));
        }

        InspectableGameModel game_model;
        PiecesModel pieces_model;
        QStringList warnings;

    private:
        // Visible itself and through every ancestor, with some size.
        [[nodiscard]] static auto
        isShown (const QQuickItem* item)
            -> bool
        {
            return item->width() > 0 && item->height() > 0 && isVisibleThroughAncestors (item);
        }

        // Text is drawn at its own size even when its box has none, as in a
        // dialog too short for its padding, so the drawn size is what counts.
        [[nodiscard]] static auto
        isTextShown (const QQuickItem* item)
            -> bool
        {
            return item->property ("paintedWidth").toDouble() > 0
                && item->property ("paintedHeight").toDouble() > 0
                && isVisibleThroughAncestors (item);
        }

        [[nodiscard]] static auto
        sceneRect (const QQuickItem* item)
            -> QRectF
        {
            return item->mapRectToScene (QRectF { 0, 0, item->width(), item->height() });
        }

        [[nodiscard]] static auto
        isVisibleThroughAncestors (const QQuickItem* item)
            -> bool
        {
            for (auto* ancestor = item; ancestor != nullptr; ancestor = ancestor->parentItem())
            {
                if (!ancestor->isVisible())
                    return false;
            }
            return true;
        }

        [[nodiscard]] auto
        itemsWithProperties (const char* first, const char* second) const
            -> QList<QQuickItem*>
        {
            QList<QQuickItem*> result;
            collectItems (window()->contentItem(), first, second, result);
            return result;
        }

        // Walks the visual tree: a Repeater's delegates are child items of
        // its parent without being its QObject children.
        static void collectItems ( // NOLINT(misc-no-recursion)
            QQuickItem* item,
            const char* first,
            const char* second,
            QList<QQuickItem*>& result
        ) {
            if (item->property (first).isValid() && item->property (second).isValid())
                result << item;

            for (auto* child : item->childItems())
                collectItems (child, first, second, result);
        }

        QQmlApplicationEngine my_engine;
    };

    // Where the middle of an item is drawn, in window coordinates. The
    // middle stays meaningful when the board and the pieces are rotated.
    inline auto
    drawnAt (const QQuickItem* item)
        -> QPointF
    {
        return item->mapToScene (QPointF { item->width() / 2, item->height() / 2 });
    }

    inline auto
    drawnOn (const Application& app, const QQuickItem* piece, const char* coord_text)
        -> bool
    {
        auto expected = drawnAt (app.squareAt (coord_text));
        auto actual = drawnAt (piece);
        return qAbs (actual.x() - expected.x()) < 0.5 && qAbs (actual.y() - expected.y()) < 0.5;
    }
}
