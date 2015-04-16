
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
using ugdk::action::mode3d::component::ContactPointVector;

namespace {

ugdk::action::mode3d::Scene3D *ourscene;

struct {
   double angle;
} player;

} // unnamed namespace

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
    auto headEnt = mSceneMgr->createEntity(name, "Mage.mesh");
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
    headData.apply_orientation = false;
    head->AddComponent(make_shared<PhysicsBody>(*ourscene->physics(), headData));
    head->component<Body>()->set_damping(.8, .8);
    return head;
}

shared_ptr<Element> createWall(const std::string& name, const Ogre::Vector3& dir, double dist, double width = AREA_RANGE, double height = AREA_RANGE) {
    Ogre::SceneManager *mSceneMgr = ourscene->manager();
    // Element
    auto wall = ourscene->AddElement();
    // View
    Ogre::Plane plane(dir, dist);
    auto mesh_ptr = Ogre::MeshManager::getSingleton().createPlane(name, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
        plane, width, height, 5, 5, true, 1, 5, 5, dir.perpendicular());
    const std::string wat = name + "Entity";
    Ogre::Entity* wallEnt = mSceneMgr->createEntity(wat, mesh_ptr);
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
    wall->component<Body>()->set_friction(20);
    return wall;
}

int main(int argc, char* argv[]) {
    using Ogre::Vector3;

    ugdk::system::Configuration config;
    config.base_path = "assets/";
    ugdk::system::Initialize(config);
    ourscene = new ugdk::action::mode3d::Scene3D(btVector3(0.0, -10.0, 0.0));
    
    ourscene->physics()->set_debug_draw_enabled(true);
    ourscene->ShowFrameStats();

    {
        player.angle = 0.0;
        weak_ptr<Element> head1 = createOgreHead("Head");
        weak_ptr<Element> head2 = createOgreHead("Head2", true);
        auto body2 = head2.lock()->component<Body>();
        body2->Translate(0, 0, 80);
        body2->set_angular_factor(0.0, 0.0, 0.0);

        body2->AddCollisionAction(CollisionGroup::HEADS, 
        [](const ElementPtr& self, const ElementPtr& target, const ContactPointVector& pts) {
            cout << "CARAS COLIDINDO MANO (" << pts.size() << ")" << endl;
        });

        weak_ptr<Element> wall = createWall("ground", Vector3::UNIT_Y, -5.0);

        ourscene->camera()->AttachTo(*head2.lock());
        ourscene->camera()->SetParameters(Vector3::UNIT_Y*10.0, 5000.0);
        ourscene->camera()->SetDistance(80.0);
        ourscene->camera()->Rotate(0.0, Ogre::Degree(20.0).valueDegrees());
        
        ourscene->AddTask(ugdk::system::Task(
        [head2, body2](double dt) {
            auto& keyboard = ugdk::input::manager()->keyboard();
            Vector3 move = Vector3::ZERO;
            if (keyboard.IsDown(ugdk::input::Scancode::RIGHT))
                player.angle -= dt*135.0;
            else if (keyboard.IsDown(ugdk::input::Scancode::LEFT))
                player.angle += dt*135.0;
            if (keyboard.IsDown(ugdk::input::Scancode::UP))
                move.z += 1.0;
            else if (keyboard.IsDown(ugdk::input::Scancode::DOWN))
                move.z += -1.0;

            Ogre::Quaternion rot(Ogre::Degree(player.angle), Vector3::UNIT_Y);
            move.normalise();
            move = rot * move;
            move.y = 0.0;
            move.normalise();

            body2->ApplyImpulse(move * 200);
            Ogre::SceneNode &node = head2.lock()->node();
            node.setOrientation(rot);
        }));

        ourscene->event_handler().AddListener<ugdk::input::KeyPressedEvent>(
            [] (const ugdk::input::KeyPressedEvent& ev) -> void {
                if (ev.scancode == ugdk::input::Scancode::ESCAPE)
                    ourscene->Finish();
            });
        //ourscene->event_handler().AddListener<ugdk::input::MouseMotionEvent>(
        //    [] (const ugdk::input::MouseMotionEvent& ev) -> void {
        //        ourscene->camera()->Rotate(-ev.motion.x, ev.motion.y);
        //    });

        ourscene->manager()->setAmbientLight(Ogre::ColourValue(.7, .7, .7));

        ugdk::system::PushScene(unique_ptr<ugdk::action::Scene>(ourscene));

    }

    ugdk::system::Run();
    ugdk::system::Release();
    return 0;
}
