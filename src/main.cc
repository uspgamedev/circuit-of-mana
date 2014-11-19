
#include <ugdk/system/engine.h>
#include <ugdk/action/3D/camera.h>
#include <ugdk/input/events.h>
#include <bulletworks/object.h>
#include <bulletworks/physicscene.h>
#include <memory>

#include <OgreCamera.h>
#include <OgreEntity.h>
#include <OgreLogManager.h>
#include <OgreRoot.h>
#include <OgreViewport.h>
#include <OgreSceneManager.h>
#include <OgreRenderWindow.h>
#include <OgreConfigFile.h>
#include <OgreMeshManager.h>

#define AREA_RANGE 20.0

using std::unique_ptr;

bulletworks::PhysicScene *ourscene;

int main(int argc, char* argv[]) {
    ugdk::system::Configuration config;
    config.base_path = "assets/";
    ugdk::system::Initialize(config);
    ourscene = new bulletworks::PhysicScene(btVector3(0, 0, 0));
    double delay = 5.0;
    ourscene->AddTask(ugdk::system::Task(
        [&delay](double dt) {
          if ((delay -= dt) < 0)
              ourscene->Finish();
        }));

    Ogre::SceneManager *mSceneMgr = ourscene->manager();

    bulletworks::Object::PhysicsData head1data;
    auto head1 = new bulletworks::Object(mSceneMgr->createEntity("Head", "ogrehead.mesh"), head1data);
    head1->AddToScene(ourscene);
    head1->node()->setPosition(0, 0, 0);

    bulletworks::Object::PhysicsData head2data;
    auto head2 = new bulletworks::Object(mSceneMgr->createEntity("Head2", "ogrehead.mesh"), head2data);
    head2->AddToScene(ourscene);
    head2->node()->setPosition(0, 0, 80);

    ourscene->camera()->AttachTo(head2);
    //ourscene->camera()->SetDistance(50);

    ourscene->event_handler().AddListener<ugdk::input::KeyPressedEvent>(
        [head1] (const ugdk::input::KeyPressedEvent& ev) -> void {
            if (ev.scancode == ugdk::input::Scancode::RIGHT)
                head1->Translate(10.0, 0.0, 0.0);
            else if (ev.scancode == ugdk::input::Scancode::LEFT)
                head1->Translate(-10.0, 0.0, 0.0);
        });

    mSceneMgr->setAmbientLight(Ogre::ColourValue(0.6, .6, .6));

    ugdk::system::PushScene(unique_ptr<ugdk::action::Scene>(ourscene));
    ugdk::system::Run();
    ugdk::system::Release();
    return 0;
}
