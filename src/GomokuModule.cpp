// it's replaced by Node-Addon-API

// #include <node.h>
// #include <v8.h>
// #include <vector>
// #include "../includes/SearchEngine.h"

// using namespace v8;

// Handle<Value> search(const Arguments& args) {
// 	const int ERROR_RESULT = 0x100;

// 	HandleScope scope;
// 	if (args.Length() < 6) {
// 		ThrowException(Exception::TypeError(String::New("Wrong number of arguments: " + args.Length())));
// 		return scope.Close(Uint32::NewFromUnsigned(ERROR_RESULT));
// 	}

// 	// level
// 	if (!args[0]->IsUint32()) {
// 		ThrowException(Exception::TypeError(String::New("Wrong arguments")));
// 		return scope.Close(Uint32::NewFromUnsigned(ERROR_RESULT));
// 	}

// 	uint32_t level = args[0]->Uint32Value();

// 	if (level <= 0 || level > 30) {
// 		ThrowException(Exception::TypeError(String::New("Level out of range, level should in 1~30")));
// 		return scope.Close(Uint32::NewFromUnsigned(ERROR_RESULT));
// 	}

// 	// position list
// 	if (!args[1]->IsArray()) {
// 		ThrowException(Exception::TypeError(String::New("Wrong arguments")));
// 		return scope.Close(Uint32::NewFromUnsigned(ERROR_RESULT));
// 	}

// 	std::vector<uint32_t> posList;

// 	Local<Array> input = Local<Array>::Cast(args[1]);
// 	uint32_t num_locations = input->Length();

// 	for (unsigned int i = 0; i < num_locations; i++) {
// 		Local<Integer> value = Local<Integer>::Cast(input->Get(i));

// 		posList.push_back(value->IntegerValue());
// 	}

// 	// type
// 	if (!args[2]->IsUint32()) {
// 		ThrowException(Exception::TypeError(String::New("Wrong arguments")));
// 		return scope.Close(Uint32::NewFromUnsigned(ERROR_RESULT));
// 	}

// 	uint32_t type = args[2]->Uint32Value();

// 	SearchEngine* searchEngine = SearchEngine::getInstance();

// 	uint32_t result = searchEngine->search(level,
// 			posList,
// 			type);

// 	return scope.Close(Uint32::NewFromUnsigned(result));
// }

// void register_module(Handle<Object> target) {
// 	NODE_SET_METHOD(target, "search", search);
// }
// NODE_MODULE(gomoku, register_module)
