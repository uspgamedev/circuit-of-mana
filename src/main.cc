
#include <ugdk/system/engine.h>
#include <ugdk/action/3D/ogrescene.h>
#include <ugdk/action/3D/ogreentity.h>
#include <ugdk/action/3D/camera.h>
#include <ugdk/input/events.h>
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

ugdk::action::mode3d::OgreScene *ourscene;

struct mahentity : public ugdk::action::mode3d::OgreEntity {
  void set_node(Ogre::SceneNode* n) { node_ = n; }
};

int main(int argc, char* argv[]) {
    ugdk::system::Configuration config;
    config.base_path = "assets/";
    ugdk::system::Initialize(config);
    ourscene = new ugdk::action::mode3d::OgreScene;
    double delay = 5.0;
    ourscene->AddTask(ugdk::system::Task(
        [&delay](double dt) {
          if ((delay -= dt) < 0)
              ourscene->Finish();
        }));

    Ogre::SceneManager *mSceneMgr = ourscene->manager();
    auto head = mSceneMgr->createEntity("Head", "ogrehead.mesh");
    auto head2 = mSceneMgr->createEntity("Head2", "ogrehead.mesh");
    auto node = mSceneMgr->getRootSceneNode()->createChildSceneNode("HeadNode");
    auto node2 = mSceneMgr->getRootSceneNode()->createChildSceneNode("HeadNode2");
    node->attachObject(head);
    node2->attachObject(head2);
    node2->setPosition(Ogre::Vector3(0, 0, 80));

    mahentity* mah = new mahentity;
    mah->set_node(node2);
    ourscene->camera()->AttachTo(mah);

    mSceneMgr->setAmbientLight(Ogre::ColourValue(1.0, 1.0, 1.0));

    ugdk::system::PushScene(unique_ptr<ugdk::action::Scene>(ourscene));
    ugdk::system::Run();
    ugdk::system::Release();
    return 0;
}
