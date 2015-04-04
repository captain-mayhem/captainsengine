#include <fbxsdk.h>

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

  return 0;
}
