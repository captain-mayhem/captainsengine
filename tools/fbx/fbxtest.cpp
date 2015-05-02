#include <fbxsdk.h>


void handleMesh(FbxMesh* mesh){
  printf("Mesh %s\n", mesh->GetName());
}

void traverseNode(FbxNode* node){

  if (node->GetNodeAttribute() != NULL){
    FbxNodeAttribute::EType type = node->GetNodeAttribute()->GetAttributeType();
    switch (type){
    case FbxNodeAttribute::eMesh:
      handleMesh((FbxMesh*)node);
      break;
    default:
      printf("node %s unhandled\n", node->GetNodeAttribute()->GetName());
      break;
    }
  }

  for (int i = 0; i < node->GetChildCount(); ++i){
    traverseNode(node->GetChild(i));
  }
}

int main(int argc, char** argv){
  if (argc < 2){
    printf("Usage: %s <fbx-file>\n", argv[0]);
    return -1;
  }
  FbxManager* fbx = FbxManager::Create();
  FbxIOSettings* ios = FbxIOSettings::Create(fbx, IOSROOT);
  fbx->SetIOSettings(ios);

  FbxImporter* importer = FbxImporter::Create(fbx, "");
  bool ret = importer->Initialize(argv[1], -1, fbx->GetIOSettings());
  if (!ret){
    printf("Cannot import %s: %s\n", argv[1], importer->GetStatus().GetErrorString());
    return -1;
  }

  FbxScene* scene = FbxScene::Create(fbx, "scene");
  importer->Import(scene);
  importer->Destroy();

  /*for (int i = 0; i < scene->GetVideoCount(); ++i){
    FbxVideo* vid = scene->GetVideo(i);
    bool ret = vid->ContentIsLoaded();
    vid->ContentWriteTo(csc);
    vid = vid;
  }*/

  for (int i = 0; i < scene->GetTextureCount(); ++i){
    FbxTexture* tex = scene->GetTexture(i);
    tex = tex;
  }

  FbxNode* node = scene->GetRootNode();
  traverseNode(node);

  return 0;
}
