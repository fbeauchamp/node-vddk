#include <napi.h>
#include <string>
#include <cstring>
#include <dlfcn.h>
#include "vixDiskLib.h"

// Define version constants if not provided by VDDK 8 headers
#ifndef VIXDISKLIB_VERSION_MAJOR
#define VIXDISKLIB_VERSION_MAJOR 8
#endif
#ifndef VIXDISKLIB_VERSION_MINOR
#define VIXDISKLIB_VERSION_MINOR 0
#endif

class VddkWrapper : public Napi::ObjectWrap<VddkWrapper> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports) {
        Napi::Function func = DefineClass(env, "VddkWrapper", {
            InstanceMethod("connect", &VddkWrapper::Connect),
            InstanceMethod("disconnect", &VddkWrapper::Disconnect),
            InstanceMethod("openDisk", &VddkWrapper::OpenDisk),
            InstanceMethod("closeDisk", &VddkWrapper::CloseDisk),
            InstanceMethod("read", &VddkWrapper::Read),
            InstanceMethod("write", &VddkWrapper::Write)
        });

        exports.Set("VddkWrapper", func);
        return exports;
    }

    VddkWrapper(const Napi::CallbackInfo& info) : Napi::ObjectWrap<VddkWrapper>(info) {
        Napi::Env env = info.Env();
        dlopen("/vagrant/disklib/lib64/libcrypto.so.1.0.2", RTLD_NOW | RTLD_DEEPBIND);
        dlopen("/vagrant/disklib/lib64/libssl.so.1.0.2", RTLD_NOW | RTLD_DEEPBIND);
        dlopen("/vagrant/disklib/lib64/libvddkVimAccess.so", RTLD_NOW | RTLD_DEEPBIND);
        // Initialize VDDK with explicit version
        VixError err = VixDiskLib_InitEx(
            VIXDISKLIB_VERSION_MAJOR,
            VIXDISKLIB_VERSION_MINOR,
            NULL, NULL, NULL, NULL, NULL);
        printf("\nINIT!\n");
        if (VIX_FAILED(err)) {
            std::string errorMsg = "Failed to initialize VDDK: ";
            errorMsg += VixDiskLib_GetErrorText(err, NULL);
            Napi::Error::New(env, errorMsg.c_str()).ThrowAsJavaScriptException();
            return;
        }
    }

    ~VddkWrapper() {
        printf("\nCLOSED!\n");
        if (diskHandle) VixDiskLib_Close(diskHandle);
        if (connection) VixDiskLib_Disconnect(connection);
        VixDiskLib_Exit();
    }

private:
    VixDiskLibConnection connection = NULL;
    VixDiskLibHandle diskHandle = NULL;

    Napi::Value Connect(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();

        if (info.Length() < 4) {
            Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
            return env.Null();
        }

        std::string server = info[0].As<Napi::String>();
        std::string thumbprint = info[1].As<Napi::String>();
        std::string username = info[2].As<Napi::String>();
        std::string password = info[3].As<Napi::String>();

        VixDiskLibConnectParams *cnxParams = VixDiskLib_AllocateConnectParams();
        cnxParams->vmxSpec = strdup(server.c_str());
        cnxParams->serverName = strdup(server.c_str());
        cnxParams->credType = VIXDISKLIB_CRED_UID;
        cnxParams->creds.uid.userName = strdup(username.c_str());
        cnxParams->creds.uid.password = strdup(password.c_str());
        cnxParams->thumbPrint = strdup(thumbprint.c_str());
        VixError err = VixDiskLib_Connect(cnxParams, &connection);


        if (VIX_FAILED(err)) {
            std::string errorMsg = "Failed to connect: ";
            errorMsg += VixDiskLib_GetErrorText(err, NULL);
            Napi::Error::New(env, errorMsg.c_str()).ThrowAsJavaScriptException();
            return env.Null();
        }

        return Napi::Boolean::New(env, true);
    }

    Napi::Value Disconnect(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();

        if (connection) {
            VixDiskLib_Disconnect(connection);
            connection = NULL;
        }

        return Napi::Boolean::New(env, true);
    }

    Napi::Value OpenDisk(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();

        if (info.Length() < 1) {
            Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
            return env.Null();
        }

        std::string path = info[0].As<Napi::String>();

        // VDDK 8 uses transport modes directly in Open call
        VixError err = VixDiskLib_Open(connection,
                                      path.c_str(),
                                      VIXDISKLIB_FLAG_OPEN_UNBUFFERED | VIXDISKLIB_FLAG_OPEN_SINGLE_LINK,
                                      &diskHandle);

        if (VIX_FAILED(err)) {
            std::string errorMsg = "Failed to open disk: ";
            errorMsg += VixDiskLib_GetErrorText(err, NULL);
            Napi::Error::New(env, errorMsg.c_str()).ThrowAsJavaScriptException();
            return env.Null();
        }

        return Napi::Boolean::New(env, true);
    }

    Napi::Value CloseDisk(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();

        if (diskHandle) {
            VixDiskLib_Close(diskHandle);
            diskHandle = NULL;
        }

        return Napi::Boolean::New(env, true);
    }

    Napi::Value Read(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();

        if (info.Length() < 2) {
            Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
            return env.Null();
        }

        VixDiskLibSectorType sectorNum = info[0].As<Napi::Number>().Uint32Value();
        uint32_t sectorCount = info[1].As<Napi::Number>().Uint32Value();
        uint32_t length = sectorCount * VIXDISKLIB_SECTOR_SIZE;

        // Use unsigned char buffer as required by VDDK 8
        uint8_t* buffer = new uint8_t[length];
        VixError err = VixDiskLib_Read(diskHandle, sectorNum, sectorCount, buffer);

        if (VIX_FAILED(err)) {
            delete[] buffer;
            std::string errorMsg = "Failed to read: ";
            errorMsg += VixDiskLib_GetErrorText(err, NULL);
            Napi::Error::New(env, errorMsg.c_str()).ThrowAsJavaScriptException();
            return env.Null();
        }

        // Create buffer from unsigned char data
        Napi::Buffer<uint8_t> result = Napi::Buffer<uint8_t>::Copy(env, buffer, length);
        delete[] buffer;
        return result;
    }

    Napi::Value Write(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();

        if (info.Length() < 2) {
            Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
            return env.Null();
        }

        VixDiskLibSectorType sectorNum = info[0].As<Napi::Number>().Uint32Value();
        Napi::Buffer<uint8_t> buffer = info[1].As<Napi::Buffer<uint8_t>>();
        uint32_t sectorCount = buffer.Length() / VIXDISKLIB_SECTOR_SIZE;

        if (buffer.Length() % VIXDISKLIB_SECTOR_SIZE != 0) {
            Napi::Error::New(env, "Buffer size must be multiple of sector size (512)").ThrowAsJavaScriptException();
            return env.Null();
        }

        VixError err = VixDiskLib_Write(diskHandle, sectorNum, sectorCount, buffer.Data());

        if (VIX_FAILED(err)) {
            std::string errorMsg = "Failed to write: ";
            errorMsg += VixDiskLib_GetErrorText(err, NULL);
            Napi::Error::New(env, errorMsg.c_str()).ThrowAsJavaScriptException();
            return env.Null();
        }

        return Napi::Boolean::New(env, true);
    }
};

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    return VddkWrapper::Init(env, exports);
}

NODE_API_MODULE(vddk, Init)
