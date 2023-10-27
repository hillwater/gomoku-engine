#include <napi.h>
#include <vector>
#include "../includes/SearchEngine.h"

Napi::Value Search(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 3) {
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
        posList.push_back(pos);
    }

    // type
    if (!info[2].IsNumber()) {
        Napi::TypeError::New(env, "Wrong type arguments").ThrowAsJavaScriptException();
        return env.Null();
    }

    uint32_t type = info[2].As<Napi::Number>().Int32Value();

    SearchEngine* searchEngine = SearchEngine::getInstance();

    uint32_t result = searchEngine->search(level,posList,type);

    return Napi::Number::New(env, result);
}

Napi::Value GenerateMoves(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    uint32_t expandLevel = info[0].As<Napi::Number>().Int32Value();

    // position list
    Napi::Array arr = info[1].As<Napi::Array>();
    std::vector<uint32_t> posList;
    uint32_t num_locations = arr.Length();
    for (uint32_t i = 0; i < num_locations; i++) {
        int pos = arr.Get(static_cast<napi_value>(Napi::Number::New(info.Env(),i))).As<Napi::Number>().Int32Value();
        posList.push_back(pos);
    }

    uint32_t type = info[2].As<Napi::Number>().Int32Value();
    
    uint32_t branchFactor = info[3].As<Napi::Number>().Int32Value();

    SearchEngine* searchEngine = SearchEngine::getInstance();

    std::vector<uint32_t> nextMoves;
    searchEngine->generateMovesForOpenBook(expandLevel, posList,type,branchFactor, nextMoves);

    Napi::Array result = Napi::Array::New(env);
    for (uint32_t i = 0; i < nextMoves.size(); i++) {
        result.Set(Napi::Number::New(env,i),Napi::Number::New(env,nextMoves[i]));
    }

    return result;
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set(Napi::String::New(env, "search"), Napi::Function::New(env, Search));
    exports.Set(Napi::String::New(env, "generateMoves"), Napi::Function::New(env, GenerateMoves));
    return exports;
}

NODE_API_MODULE(addon, Init)