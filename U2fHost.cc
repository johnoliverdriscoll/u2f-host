#include <nan.h>
#include "u2f-host.h"

#undef INIT_FLAGS
#ifdef NDEBUG
#   define INIT_FLAGS (U2FH_DEBUG)
#else
#   define INIT_FLAGS (0)
#endif

namespace u2fh {

// Pointer to a libu2f-host command function
typedef u2fh_rc (*u2fh_command)(
    u2fh_devs * devs,
    const char *challenge,
    const char *origin,
    char **response,
    u2fh_cmdflags flags
);

// Wrapped module object
class U2fHost : public Nan::ObjectWrap {

public:

    // Class initializer
    static NAN_MODULE_INIT(Init) {
        // Init libu2f-host
        u2fh_initflags initflags = (u2fh_initflags) INIT_FLAGS;
        u2fh_rc rc;
        rc = u2fh_global_init(initflags);
        if (rc != U2FH_OK) {
            Nan::ThrowError(u2fh_strerror(rc));
            return;
        }
        node::AtExit(Destroy);
        // Add class constants
        Nan::Set(
            target,
            Nan::New<v8::String>("U2FH_REQUEST_USER_PRESENCE").ToLocalChecked(),
            Nan::New<v8::Uint32>((uint32_t)U2FH_REQUEST_USER_PRESENCE)
        );
        // Add factory method
        Nan::Set(
            target,
            Nan::New<v8::String>("discover").ToLocalChecked(),
            Nan::GetFunction(Nan::New<v8::FunctionTemplate>(Discover)).ToLocalChecked()
        );
        // Create instance prototype
        v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
        tpl->SetClassName(Nan::New("U2fHost").ToLocalChecked());
        tpl->InstanceTemplate()->SetInternalFieldCount(1);
        Nan::SetPrototypeMethod(tpl, "register", Register);
        Nan::SetPrototypeMethod(tpl, "authenticate", Authenticate);
        constructor().Reset(Nan::GetFunction(tpl).ToLocalChecked());
    }

    // Module destroy
    static void Destroy(void*) {
        u2fh_global_done();
    }

    // Discover new devices
    static NAN_METHOD(Discover) {
        u2fh_devs *devs;
        u2fh_rc rc;
        // Init device list
        rc = u2fh_devs_init(&devs);
        if (rc != U2FH_OK) {
            Nan::ThrowError(u2fh_strerror(rc));
            return;
        }
        // Get actual devices pointer
        rc = u2fh_devs_discover(devs, NULL);
        if (rc != U2FH_OK) {
            Nan::ThrowError(u2fh_strerror(rc));
            return;
        }
        // Return new U2fHost instance wrapping devs
        v8::Local<v8::Value> devices = Nan::New<v8::External>(devs);
        v8::Local<v8::Function> cons = Nan::New(constructor());
        info.GetReturnValue().Set(Nan::NewInstance(cons, 1, &devices).ToLocalChecked());
    }

private:

    // Native object constructor
    explicit U2fHost(u2fh_devs *devices) : devs(devices) {}

    // Native object deinitializer
    ~U2fHost() {
        u2fh_devs_done(this->devs);
    }

    // Object constructor
    static NAN_METHOD(New) {
        if (!info.IsConstructCall()) {
            // Invoked as plain function `U2fHost(...)`, turn into construct call.
            v8::Local<v8::Function> cons = Nan::New(constructor());
            v8::Local<v8::Value> devices = info[0];
            info.GetReturnValue().Set(Nan::NewInstance(cons, 1, &devices).ToLocalChecked());
            return;
        }
        // Get native devs pointer from constructor argument
        u2fh_devs *devs = (u2fh_devs*)info[0].As<v8::External>()->Value();
        U2fHost *obj = new U2fHost(devs);
        // Return wrapped addon
        obj->Wrap(info.This());
        info.GetReturnValue().Set(info.This());
    }

    // Validate arguments to #register and #authenticate
    static bool validate(const Nan::FunctionCallbackInfo<v8::Value> &info) {
        if (info.Length() < 2) {
            Nan::ThrowError("Expected at least 2 arguments");
        } else if (!info[0]->IsString()) {
            Nan::ThrowError("Expected argument 1 to be a String");
        } else if (!info[1]->IsString()) {
            Nan::ThrowError("Expected argument 2 to be a String");
        } else if (info.Length() > 2 && !info[2]->IsUint32()) {
            Nan::ThrowError("Expected argument 3 to be a Uint32");
        } else if (info.Length() > 3) {
            Nan::ThrowError("Expected at most 3 arguments");
        } else {
            return true;
        }
        return false;
    }

    // Send a register or authenticate command to U2F devices
    static void send(const Nan::FunctionCallbackInfo<v8::Value> &info, u2fh_command command) {
        U2fHost *obj = Nan::ObjectWrap::Unwrap<U2fHost>(info.Holder());
        u2fh_cmdflags cmdflags;
        u2fh_rc rc;
        char *response = NULL;
        // Validate arguments
        if (!validate(info)) {
            return;
        }
        // Get cmdflags or use defaults
        if (info.Length() > 2) {
            cmdflags = (u2fh_cmdflags)info[2]->Uint32Value();
        } else {
            cmdflags = U2FH_REQUEST_USER_PRESENCE;
        }
        // Decode arguments to native strings
        char challenge[info[0]->ToString()->Length()];
        char origin[info[1]->ToString()->Length()];
        Nan::DecodeWrite(challenge, sizeof(challenge), info[0]);
        Nan::DecodeWrite(origin, sizeof(origin), info[1]);
        // Send command
        rc = command(obj->devs, challenge, origin, &response, cmdflags);
        if (rc != U2FH_OK) {
            Nan::ThrowError(u2fh_strerror(rc));
            return;
        }
        info.GetReturnValue().Set(Nan::Encode(response, strlen(response)));
        free(response);
    }

    // Send register command
    static NAN_METHOD(Register) {
        send(info, u2fh_register);
    }

    // Send authenticate command
    static NAN_METHOD(Authenticate) {
        send(info, u2fh_authenticate);
    }

    static inline Nan::Persistent<v8::Function> & constructor() {
        static Nan::Persistent<v8::Function> my_constructor;
        return my_constructor;
    }

    // Native devices pointer from u2fh_devs_discover
    u2fh_devs *devs;
};

NODE_MODULE(U2fHost, U2fHost::Init)

}
