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
        GameModel* game_model_instance = nullptr;
        PiecesModel* pieces_model_instance = nullptr;

        // The instance stays owned by C++; the engine must not delete it.
        template <typename T>
        auto instanceFor (T* instance, QJSEngine* js_engine)
            -> T*
        {
            expects (instance != nullptr);
            expects (js_engine->thread() == instance->thread());
            QJSEngine::setObjectOwnership (instance, QJSEngine::CppOwnership);
            return instance;
        }
    }

    void GameModelSingleton::setInstance (GameModel* instance)
    {
        game_model_instance = instance;
    }

    auto GameModelSingleton::create ([[maybe_unused]] QQmlEngine* engine, QJSEngine* js_engine)
        -> GameModel*
    {
        return instanceFor (game_model_instance, js_engine);
    }

    void PiecesModelSingleton::setInstance (PiecesModel* instance)
    {
        pieces_model_instance = instance;
    }

    auto PiecesModelSingleton::create ([[maybe_unused]] QQmlEngine* engine, QJSEngine* js_engine)
        -> PiecesModel*
    {
        return instanceFor (pieces_model_instance, js_engine);
    }
}
