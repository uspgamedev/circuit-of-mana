
#include <ugdk/system/engine.h>
#include <ugdk/action/3D/camera.h>
#include <ugdk/input/events.h>
#include <ugdk/input/module.h>
#include <ugdk/action/3D/element.h>
#include <ugdk/action/3D/scene3d.h>
#include <ugdk/action/3D/physics.h>
#include <ugdk/action/3D/component/physicsbody.h>
#include <ugdk/action/3D/component/view.h>
#include <BtOgreGP.h>
#include <memory>
#include <iostream>

#include <OgreCamera.h>
#include <OgreEntity.h>
#include <OgreLogManager.h>
#include <OgreRoot.h>
#include <OgreViewport.h>
#include <OgreSceneManager.h>
#include <OgreRenderWindow.h>
#include <OgreConfigFile.h>
#include <OgreMeshManager.h>

#define AREA_RANGE 200.0

using std::weak_ptr;
using std::shared_ptr;
using std::unique_ptr;
using std::make_shared;
using std::cout;
using std::endl;
using ugdk::action::mode3d::Element;
using ugdk::action::mode3d::component::PhysicsBody;
using ugdk::action::mode3d::component::Body;
using ugdk::action::mode3d::component::View;
using ugdk::action::mode3d::component::CollisionAction;
using ugdk::action::mode3d::component::ElementPtr;
using ugdk::action::mode3d::component::ManifoldPointVector;

ugdk::action::mode3d::Scene3D *ourscene;

#define BIT(x) (1<<(x))
enum CollisionGroup {
    HEADS = BIT(6),
    WALLS = BIT(7),
    WAT2 = BIT(8),
    WAT3 = BIT(9),
    WAT4 = BIT(10)
};

shared_ptr<Element> createOgreHead(const std::string& name, bool useBox=false) {
    Ogre::SceneManager *mSceneMgr = ourscene->manager();
    // Element
    auto head = ourscene->AddElement();
    // View
    auto headEnt = mSceneMgr->createEntity(name, "Cube.mesh");
    head->AddComponent(make_shared<View>());
    head->component<View>()->AddEntity(headEnt);
    // Body
    PhysicsBody::PhysicsData headData;
    auto meshShapeConv = BtOgre::StaticMeshToShapeConverter(headEnt);
    if (useBox)
        headData.shape = meshShapeConv.createBox();
    else
        headData.shape = meshShapeConv.createSphere();
    headData.mass = 80;
    headData.collision_group = CollisionGroup::HEADS;
    headData.collides_with = CollisionGroup::WALLS | CollisionGroup::HEADS;
    head->AddComponent(make_shared<PhysicsBody>(*ourscene->physics(), headData));
    head->component<Body>()->set_damping(.4, .4);
    return head;
}

shared_ptr<Element> createWall(const std::string& name, const Ogre::Vector3& dir, double dist, double width = AREA_RANGE, double height = AREA_RANGE) {
    Ogre::SceneManager *mSceneMgr = ourscene->manager();
    // Element
    auto wall = ourscene->AddElement();
    // View
    Ogre::Plane plane(dir, dist);
    Ogre::MeshManager::getSingleton().createPlane(name, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
        plane, width, height, 5, 5, true, 1, 5, 5, dir.perpendicular());
    const std::string wat = name + "Entity";
    Ogre::Entity* wallEnt = mSceneMgr->createEntity(wat, name);
    wallEnt->setMaterialName("Ogre/Tusks");
    wall->AddComponent(make_shared<View>());
    wall->component<View>()->AddEntity(wallEnt);
    // Body
    PhysicsBody::PhysicsData wallData;
    wallData.shape = new btStaticPlaneShape(btVector3(dir.x, dir.y, dir.z), dist);
    wallData.mass = 0;
    wallData.collision_group = CollisionGroup::WALLS;
    wallData.collides_with = CollisionGroup::HEADS;
    wall->AddComponent(make_shared<PhysicsBody>(*ourscene->physics(), wallData));
    wall->component<Body>()->set_friction(1.7);
    return wall;
}

int main(int argc, char* argv[]) {
    ugdk::system::Configuration config;
    config.base_path = "assets/";
    ugdk::system::Initialize(config);
    ourscene = new ugdk::action::mode3d::Scene3D;
    
    ourscene->physics()->set_debug_draw_enabled(true);
    ourscene->ShowFrameStats();

    {
        weak_ptr<Element> head1 = createOgreHead("Head");
        weak_ptr<Element> head2 = createOgreHead("Head2", true);
        auto body2 = head2.lock()->component<Body>();
        body2->Translate(0, 0, 80);
        body2->set_angular_factor(0.0, 0.0, 0.0);

        body2->AddCollisionAction(CollisionGroup::HEADS, 
        [](const ElementPtr& self, const ElementPtr& target, const ManifoldPointVector& pts) {
            cout << "CARAS COLIDINDO MANO (" << pts.size() << ")" << endl;
        });

        ourscene->camera()->AttachTo(*head2.lock());
        ourscene->camera()->SetParameters(Ogre::Vector3::ZERO, 5000);
        ourscene->camera()->SetDistance(100);

        createWall("ground", Ogre::Vector3::UNIT_Y, -(AREA_RANGE / 2));
        
        ourscene->AddTask(ugdk::system::Task(
        [body2](double dt) {
            auto& keyboard = ugdk::input::manager()->keyboard();
            Ogre::Vector3 move = Ogre::Vector3::ZERO;
            if (keyboard.IsDown(ugdk::input::Scancode::D))
                move.x += 1.0;
            else if (keyboard.IsDown(ugdk::input::Scancode::A))
                move.x += -1.0;
            if (keyboard.IsDown(ugdk::input::Scancode::W))
                move.z += -1.0;
            else if (keyboard.IsDown(ugdk::input::Scancode::S))
                move.z += 1.0;

            move.normalise();
            move = ourscene->camera()->actual_orientation() * move;
            move.y = 0.0;
            move.normalise();

            body2->Move((move * 10));
        }));

        ourscene->event_handler().AddListener<ugdk::input::KeyPressedEvent>(
            [] (const ugdk::input::KeyPressedEvent& ev) -> void {
                if (ev.scancode == ugdk::input::Scancode::ESCAPE)
                    ourscene->Finish();
            });
        ourscene->event_handler().AddListener<ugdk::input::MouseMotionEvent>(
            [] (const ugdk::input::MouseMotionEvent& ev) -> void {
                ourscene->camera()->Rotate(-ev.motion.x, -ev.motion.y);
            });

        ourscene->manager()->setAmbientLight(Ogre::ColourValue(0.7, .7, .7));

        ugdk::system::PushScene(unique_ptr<ugdk::action::Scene>(ourscene));

    }

    ugdk::system::Run();
    ugdk::system::Release();
    return 0;
}
