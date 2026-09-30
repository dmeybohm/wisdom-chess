#include <QJSEngine>

#include "wisdom-chess/engine/global.hpp"
#include "wisdom-chess/ui/qml/main/qml_singletons.hpp"

using namespace wisdom;

namespace wisdom::ui::qml
{
    // The engine's types, not the wisdom::ui mirrors QML sees.
    using wisdom::Color;
    using wisdom::Player;

    namespace
    {
        nullable<GameModel> game_model_instance;
        nullable<PiecesModel> pieces_model_instance;

        // The instance stays owned by C++; the engine must not delete it.
        template <typename T>
        auto instanceFor (nullable<T> maybe_instance, QJSEngine* js_engine)
            -> nonnull<T>
        {
            auto instance = maybe_instance.value();
            EXPECTS( js_engine->thread() == instance->thread() );
            QJSEngine::setObjectOwnership (instance, QJSEngine::CppOwnership);
            return instance;
        }
    }

    void GameModelSingleton::setInstance (nonnull<GameModel> instance)
    {
        game_model_instance = instance;
    }

    auto
    GameModelSingleton::create (
        [[maybe_unused]] QQmlEngine* engine,
        QJSEngine* js_engine
    ) noexcept
        -> GameModel* // lint-allow(raw-pointer): QML's singleton factory
    {
        return instanceFor (game_model_instance, js_engine).get();
    }

    void PiecesModelSingleton::setInstance (nonnull<PiecesModel> instance)
    {
        pieces_model_instance = instance;
    }

    auto
    PiecesModelSingleton::create (
        [[maybe_unused]] QQmlEngine* engine,
        QJSEngine* js_engine
    ) noexcept
        -> PiecesModel* // lint-allow(raw-pointer): QML's singleton factory
    {
        return instanceFor (pieces_model_instance, js_engine).get();
    }
}
