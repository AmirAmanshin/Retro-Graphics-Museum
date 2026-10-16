#include "Engine/Engine.hpp"
#include "Levels/Level1.hpp"

int main() {
    Engine engine(1270, 720, "Retro Graphics Museum");
    
    if (engine.init()) {
        engine.changeLevel(new Level1(&engine));
        engine.run();
    }
    
    return 0;
}