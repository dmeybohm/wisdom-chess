#include <QTest>

#include "wisdom-chess/ui/qml/main/game_settings.hpp"
#include "wisdom-chess/ui/qml/main/ui_settings.hpp"
#include "wisdom-chess/ui/qml/main/ui_types.hpp"

namespace ui = wisdom::ui;

namespace
{
    // QML writes a gadget's MEMBER properties through the meta-object.
    template <typename Gadget, typename Value>
    auto
    writeProperty (Gadget& gadget, const char* name, Value value)
        -> bool
    {
        const auto& meta_object = Gadget::staticMetaObject;
        auto index = meta_object.indexOfProperty (name);
        if (index < 0)
            return false;
        return meta_object.property (index).writeOnGadget (&gadget, QVariant::fromValue (value));
    }
}

class SettingsTest : public QObject
{
    Q_OBJECT

private slots:
    void gameSettingsDefaults()
    {
        GameSettings settings;

        QVERIFY( settings.whitePlayer() == ui::Player::Human );
        QVERIFY( settings.blackPlayer() == ui::Player::Computer );
        QCOMPARE( settings.maxDepth(), wisdom::Default_Max_Depth / 2 );
        QCOMPARE( settings.maxSearchTime(), wisdom::Default_Max_Search_Seconds );
        QCOMPARE( settings.debugLogging(), false );
    }

    void gameSettingsPropertiesAreWritable()
    {
        GameSettings settings;

        QVERIFY( writeProperty (settings, "whitePlayer", ui::Player::Computer) );
        QVERIFY( writeProperty (settings, "blackPlayer", ui::Player::Human) );
        QVERIFY( writeProperty (settings, "maxDepth", 5) );
        QVERIFY( writeProperty (settings, "maxSearchTime", 11) );
        QVERIFY( writeProperty (settings, "debugLogging", true) );

        QVERIFY( settings.whitePlayer() == ui::Player::Computer );
        QVERIFY( settings.blackPlayer() == ui::Player::Human );
        QCOMPARE( settings.maxDepth(), 5 );
        QCOMPARE( settings.maxSearchTime(), 11 );
        QCOMPARE( settings.debugLogging(), true );
    }

    void gameSettingsEquality_data()
    {
        QTest::addColumn<QString> ("property");
        QTest::addColumn<QVariant> ("value");

        QTest::newRow ("whitePlayer") << "whitePlayer" << QVariant::fromValue (ui::Player::Computer);
        QTest::newRow ("blackPlayer") << "blackPlayer" << QVariant::fromValue (ui::Player::Human);
        QTest::newRow ("maxDepth") << "maxDepth" << QVariant { 1 };
        QTest::newRow ("maxSearchTime") << "maxSearchTime" << QVariant { 30 };
        QTest::newRow ("debugLogging") << "debugLogging" << QVariant { true };
    }

    // Each property on its own makes two settings unequal.
    void gameSettingsEquality()
    {
        QFETCH( QString, property );
        QFETCH( QVariant, value );

        GameSettings original;
        GameSettings changed;
        QVERIFY( original == changed );

        QVERIFY( writeProperty (changed, property.toUtf8().constData(), value) );

        QVERIFY( original != changed );
        QVERIFY( !(original == changed) );
    }

    // Without braces: the member has to initialize itself.
    void uiSettingsStartUnflipped()
    {
        UISettings settings;

        QCOMPARE( settings.flipped(), false );
    }

    void uiSettings()
    {
        UISettings settings {};
        UISettings flipped {};

        QCOMPARE( settings.flipped(), false );
        QVERIFY( settings == flipped );

        QVERIFY( writeProperty (flipped, "flipped", true) );

        QCOMPARE( flipped.flipped(), true );
        QVERIFY( settings != flipped );
    }

    void colorsRoundTrip()
    {
        static_assert (ui::mapColor (wisdom::Color::White) == ui::Color::White);
        static_assert (ui::mapColor (wisdom::Color::Black) == ui::Color::Black);
        static_assert (ui::mapColor (ui::Color::White) == wisdom::Color::White);
        static_assert (ui::mapColor (ui::Color::Black) == wisdom::Color::Black);
    }

    void playersRoundTrip()
    {
        static_assert (ui::mapPlayer (wisdom::Player::Human) == ui::Player::Human);
        static_assert (ui::mapPlayer (wisdom::Player::ChessEngine) == ui::Player::Computer);
        static_assert (ui::mapPlayer (ui::Player::Human) == wisdom::Player::Human);
        static_assert (ui::mapPlayer (ui::Player::Computer) == wisdom::Player::ChessEngine);
    }

    void piecesRoundTrip()
    {
        auto pieces = {
            wisdom::Piece::None, wisdom::Piece::Pawn, wisdom::Piece::Knight,
            wisdom::Piece::Bishop, wisdom::Piece::Rook, wisdom::Piece::Queen,
            wisdom::Piece::King,
        };

        for (auto piece : pieces)
            QVERIFY( ui::mapPiece (ui::mapPiece (piece)) == piece );

        static_assert (ui::mapPiece (wisdom::Piece::Queen) == ui::PieceType::Queen);
        static_assert (ui::mapPiece (ui::PieceType::Knight) == wisdom::Piece::Knight);
    }

    // The QML promotion dialog passes these numbers.
    void pieceTypeValues()
    {
        static_assert (static_cast<int> (ui::PieceType::None) == 0);
        static_assert (static_cast<int> (ui::PieceType::Queen) == 5);
        static_assert (static_cast<int> (ui::PieceType::King) == 6);
    }

    // QML resolves DrawByRepetitionStatus.Proposed by looking the key up in
    // this namespace's meta-object. Without the keys the comparisons in
    // Dialogs.qml are against undefined, and no draw offer ever appears.
    void theDrawStatusKeysAreVisibleToQml_data()
    {
        QTest::addColumn<QString> ("key");
        QTest::addColumn<int> ("value");

        QTest::newRow ("NotReached") << "NotReached" << static_cast<int> (ui::DrawByRepetitionStatus::NotReached);
        QTest::newRow ("Proposed") << "Proposed" << static_cast<int> (ui::DrawByRepetitionStatus::Proposed);
        QTest::newRow ("Accepted") << "Accepted" << static_cast<int> (ui::DrawByRepetitionStatus::Accepted);
        QTest::newRow ("Declined") << "Declined" << static_cast<int> (ui::DrawByRepetitionStatus::Declined);
    }

    void theDrawStatusKeysAreVisibleToQml()
    {
        QFETCH( QString, key );
        QFETCH( int, value );

        const auto& meta_object = ui::staticMetaObject;
        int found = 0;
        for (int i = 0; i < meta_object.enumeratorCount(); i++)
        {
            bool ok = false;
            auto key_value = meta_object.enumerator (i).keyToValue (key.toUtf8().constData(), &ok);
            if (ok)
            {
                QCOMPARE( key_value, value );
                found++;
            }
        }

        // Exactly one, or QML's lookup by key would be ambiguous.
        QCOMPARE( found, 1 );
    }

    void theEnumsAreVisibleToTheMetaObjectSystem()
    {
        const auto& meta_object = ui::staticMetaObject;

        QVERIFY( meta_object.indexOfEnumerator ("Color") >= 0 );
        QVERIFY( meta_object.indexOfEnumerator ("Player") >= 0 );
        QVERIFY( meta_object.indexOfEnumerator ("PieceType") >= 0 );

        auto piece_type = meta_object.enumerator (meta_object.indexOfEnumerator ("PieceType"));
        QCOMPARE( piece_type.keyToValue ("Queen"), static_cast<int> (ui::PieceType::Queen) );
    }
};

QTEST_GUILESS_MAIN( SettingsTest )

#include "settings_test.moc"
