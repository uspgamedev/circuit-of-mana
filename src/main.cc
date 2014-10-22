
#include <ugdk/system/engine.h>
#include <ugdk/action/3D/ogrescene.h>
#include <ugdk/input/events.h>
#include <memory>

using std::unique_ptr;

int main(int argc, char* argv[]) {
    ugdk::system::Configuration config;
    config.base_path = "assets/";
    ugdk::system::Initialize(config);
    ugdk::action::Scene* ourscene = new ugdk::action::mode3d::OgreScene;
    double delay = 2.0;
    ourscene->AddTask(ugdk::system::Task(
        [ourscene, &delay](double dt) {
          if ((delay -= dt) < 0)
              ourscene->Finish();
        }));
    ugdk::system::PushScene(unique_ptr<ugdk::action::Scene>(ourscene));
    ugdk::system::Run();
    ugdk::system::Release();
    return 0;
}
