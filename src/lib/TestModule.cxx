#include "../../include/lib/TestModule.h"

TestModule::TestModule(){
  str = "hogehoge";
}

TestModule::~TestModule(){
}

void TestModule::Print(){
  cout<<str<<endl;
}
