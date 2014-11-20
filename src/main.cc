
#include <ugdk/system/engine.h>
#include <ugdk/action/3D/camera.h>
#include <ugdk/input/events.h>
#include <bulletworks/object.h>
#include <bulletworks/physicscene.h>
#include <bulletworks/manager.h>
#include <BtOgreGP.h>
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

#define BIT(x) (1<<(x))
enum CollisionGroup {
    HEADS = BIT(6),
    WAT1 = BIT(7),
    WAT2 = BIT(8),
    WAT3 = BIT(9),
    WAT4 = BIT(10)
};

int main(int argc, char* argv[]) {
    ugdk::system::Configuration config;
    config.base_path = "assets/";
    ugdk::system::Initialize(config);
    ourscene = new bulletworks::PhysicScene(btVector3(0, 0, 0));
    ourscene->physics_manager()->set_debug_draw_enabled(true);

    Ogre::SceneManager *mSceneMgr = ourscene->manager();

    bulletworks::Object::PhysicsData head1data;
    auto head1ent = mSceneMgr->createEntity("Head", "ogrehead.mesh");
    auto meshShapeConv = BtOgre::StaticMeshToShapeConverter(head1ent);
    head1data.shape = meshShapeConv.createSphere();
    head1data.mass = 10;
    head1data.collision_group = CollisionGroup::HEADS;
    head1data.collides_with = CollisionGroup::HEADS;
    auto head1 = new bulletworks::Object(head1ent, head1data);
    head1->AddToScene(ourscene);
    head1->Scale(0.5, 0.5, 0.5);

    bulletworks::Object::PhysicsData head2data;
    head2data.shape = meshShapeConv.createBox();
    head2data.mass = 10;
    head2data.collision_group = CollisionGroup::HEADS;
    head2data.collides_with = CollisionGroup::HEADS;
    auto head2 = new bulletworks::Object(mSceneMgr->createEntity("Head2", "ogrehead.mesh"), head2data);
    head2->AddToScene(ourscene);
    head2->Translate(0, 0, 80);
    head2->Scale(0.5, 0.5, 0.5);

    ourscene->camera()->AttachTo(head2);
    ourscene->camera()->SetParameters(Ogre::Vector3::ZERO, 5000);
    //ourscene->camera()->SetDistance(50);

    ourscene->event_handler().AddListener<ugdk::input::KeyPressedEvent>(
        [head2] (const ugdk::input::KeyPressedEvent& ev) -> void {
            if (ev.scancode == ugdk::input::Scancode::RIGHT)
                head2->Move(10.0, 0.0, 0.0);
            else if (ev.scancode == ugdk::input::Scancode::LEFT)
                head2->Move(-10.0, 0.0, 0.0);
            else if (ev.scancode == ugdk::input::Scancode::UP)
                head2->Move(0.0, 0.0, -10.0);
            else if (ev.scancode == ugdk::input::Scancode::DOWN)
                head2->Move(0.0, 0.0, 10.0);
            else if (ev.scancode == ugdk::input::Scancode::ESCAPE)
                ourscene->Finish();
        });

    mSceneMgr->setAmbientLight(Ogre::ColourValue(0.6, .6, .6));

    ugdk::system::PushScene(unique_ptr<ugdk::action::Scene>(ourscene));
    ugdk::system::Run();
    ugdk::system::Release();
    return 0;
}
