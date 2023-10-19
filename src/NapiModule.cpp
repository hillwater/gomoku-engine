#include <napi.h>
#include <vector>
#include "../includes/SearchEngine.h"

Napi::Value Search(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 6) {
        Napi::TypeError::New(env, "Wrong number of arguments" + info.Length()).ThrowAsJavaScriptException();
        return env.Null();
    }

    // level
    if (!info[0].IsNumber()) {
        Napi::TypeError::New(env, "Wrong level arguments").ThrowAsJavaScriptException();
        return env.Null();
    }

    int level = info[0].As<Napi::Number>().Int32Value();
    if (level <= 0 || level > 30) {
        Napi::TypeError::New(env, "Level out of range, level should in 1~30").ThrowAsJavaScriptException();
        return env.Null();
    }

    // position list
    if (!info[1].IsArray()) {
        Napi::TypeError::New(env, "Wrong posList arguments").ThrowAsJavaScriptException();
        return env.Null();
    }

    Napi::Array arr = info[1].As<Napi::Array>();
    std::vector<uint32_t> posList;
    uint32_t num_locations = arr.Length();

    for (unsigned int i = 0; i < num_locations; i++) {
        int pos = arr.Get(static_cast<napi_value>(Napi::Number::New(info.Env(),i))).As<Napi::Number>().Int32Value();
        std::cout<<""<<pos<<","
        posList.push_back(pos);
    }

    // multiple core speed up flag
    if (!info[2].IsBoolean()) {
        Napi::TypeError::New(env, "Wrong useMultiCore arguments").ThrowAsJavaScriptException();
        return env.Null();
    }

    bool useMultiCore = info[2].As<Napi::Boolean>().Value();

    // multiple machine speed up flag
    if (!info[3].IsBoolean()) {
        Napi::TypeError::New(env, "Wrong useMultiMachine arguments").ThrowAsJavaScriptException();
        return env.Null();
    }

    bool useMultiMachine = info[3].As<Napi::Boolean>().Value();

    // how many machines for multiple machine speed up
    if (!info[4].IsNumber()) {
        Napi::TypeError::New(env, "Wrong machineCount arguments").ThrowAsJavaScriptException();
        return env.Null();
    }

    uint32_t machineCount = info[4].As<Napi::Number>().Int32Value();

    // type
    if (!info[5].IsNumber()) {
        Napi::TypeError::New(env, "Wrong type arguments").ThrowAsJavaScriptException();
        return env.Null();
    }

    uint32_t type = info[5].As<Napi::Number>().Int32Value();

    SearchEngine* searchEngine = SearchEngine::getInstance();

    uint32_t result = searchEngine->search(level,posList,useMultiCore,useMultiMachine,machineCount,type);

    return Napi::Number::New(env, result);
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set(Napi::String::New(env, "search"), Napi::Function::New(env, Search));
    return exports;
}

NODE_API_MODULE(addon, Init)