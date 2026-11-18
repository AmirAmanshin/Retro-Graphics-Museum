#include "Engine/Engine.hpp"
#include "Levels/MainMenuLevel.hpp"

int main() {
    Engine engine(1280, 720, "Retro Graphics Museum");
    
    if (engine.init()) {
        engine.changeLevel(new MainMenuLevel(&engine));
        engine.run();
    }
    
    return 0;
}