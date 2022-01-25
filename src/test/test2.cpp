#include "xdg-shell-client-protocol.h"
#include "presentation-time-protocol.h"

#include<cstring>
#include<vector>
#include<memory>
#include<string>

namespace 
{
struct WaylandInput
{

};
struct WaylandOutput
{
    WaylandClient      *client;
    wl_output          *output;

    int           width;
    int           height;
    int           transform;
    int32_t       scale;
    std::string   make;
    std::string   model;
    struct {
        int   x;
        int   y;
        int   width;
        int   height;
    }geometry;
};

struct WaylandWindow
{

};

struct WaylandClient
{
    wl_display                     *display;
    wl_registry                    *registry;
    wl_compositor                  *compositor;
    wl_subcompositor               *subcompositor;
    xdg_wm_base                    *xdg_wm_base;
    wl_data_device_manager         *data_device_manager;
    wl_shm                         *shm;
    
    std::vector<std::unique_ptr<WaylandWindow>> windows;
    std::vector<std::unique_ptr<WaylandInput>>  inputs;
    std::vector<std::unique_ptr<WaylandOutput>> outputs;

    uint32_t  data_device_manager_version;
    bool      valid{false};

    void WlRegistryHandler(wl_registry* wl_registry, uint32_t name,
                           const char* interface, uint32_t version);
    void WlUnRegistryHandler(wl_registry* wl_registry, uint32_t name){}

    void AddOutput(uint32_t id);
    void AddInput(uint32_t id, uint32_t version);


    static WaylandClient* Instance() {
        static WaylandClient Singleton;
        return &Singleton;
    }

    static WaylandClient* Connect() {
        auto client = WaylandClient::Instance();
        return client->valid ? client : nullptr;
    }

    WaylandClient() {
        display = wl_display_connect(nullptr);
        if (!display) {
            //ELINUX_LOG(ERROR) << "Failed to connect to the Wayland display.";
            return;
        }

        registry = wl_display_get_registry(display);
        if (!registry) {
            //ELINUX_LOG(ERROR) << "Failed to get the wayland registry.";
            return;
        }

        wl_registry_add_listener(registry, &kWlRegistryListener, this);
        wl_display_dispatch(display);
        wl_display_roundtrip(display);

        // if (data_device_manager && wl_seat_) {
        //     wl_data_device_ = wl_data_device_manager_get_data_device(
        //         wl_data_device_manager_, wl_seat_);
        //     wl_data_device_add_listener(wl_data_device_, &kWlDataDeviceListener, this);
        // }

        valid = true;
    }
};

static const wl_registry_listener kWlRegistryListener = {
    .global =
        [](void* data,
           wl_registry* wl_registry,
           uint32_t name,
           const char* interface,
           uint32_t version) {
          auto self = reinterpret_cast<WaylandClient*>(data);
          self->WlRegistryHandler(wl_registry, name, interface, version);
        },
    .global_remove =
        [](void* data, wl_registry* wl_registry, uint32_t name) {
          auto self = reinterpret_cast<WaylandClient*>(data);
          self->WlUnRegistryHandler(wl_registry, name);
        },
};

static const xdg_wm_base_listener kXdgWmBaseListener = {
    .ping = [](void* data,
               xdg_wm_base* xdg_wm_base,
               uint32_t serial) { xdg_wm_base_pong(xdg_wm_base, serial); },
};

static const wl_output_listener kWlOutputListener = {
    .geometry = [](void* data,
                   wl_output* wl_output,
                   int32_t x,
                   int32_t y,
                   int32_t physical_width,
                   int32_t physical_height,
                   int32_t subpixel,
                   const char* make,
                   const char* model,
                   int32_t output_transform) -> void {
        auto self = reinterpret_cast<WaylandOutput*>(data);
        self->make = std::string(make);
        self->model = std::string(model);
        self->geometry.x = x;
        self->geometry.y = y;
        self->width = physical_width;
        self->height = physical_height;
        self->transform = output_transform;
    },
    .mode = [](void* data,
               wl_output* wl_output,
               uint32_t flags,
               int32_t width,
               int32_t height,
               int32_t refresh) -> void {
        auto obj = reinterpret_cast<WaylandOutput*>(data);
        if (flags & WL_OUTPUT_MODE_CURRENT) {
            obj->geometry.width = width;
            obj->geometry.height = height;
        }
    },
    .done = [](void* data, wl_output* wl_output) -> void {},
    .scale = [](void* data, wl_output* wl_output, int32_t scale) -> void {
        auto obj = reinterpret_cast<WaylandOutput*>(data);
        obj->scale = scale;
    },
};



void WaylandClient::AddOutput(uint32_t id)
{
    auto obj = std::make_unique<WaylandOutput>();
    obj->client = this;

    obj->output = static_cast<decltype(obj->output)>(
        wl_registry_bind(registry, id, &wl_output_interface, 2));

    wl_output_add_listener(obj->output, &kWlOutputListener, obj.get());

    outputs.push_back(std::move(obj));
}

void WaylandClient::WlRegistryHandler(wl_registry* wl_registry,
                                      uint32_t name,
                                      const char* interface,
                                      uint32_t version) {

    if (!strcmp(interface, wl_compositor_interface.name)) {
        compositor = static_cast<decltype(compositor)>(
            wl_registry_bind(wl_registry, name, &wl_compositor_interface, 1));
    } else if (!strcmp(interface, wl_subcompositor_interface.name)) {
        subcompositor = static_cast<decltype(subcompositor)>(
            wl_registry_bind(wl_registry, name, &wl_subcompositor_interface, 1));
    } else if (!strcmp(interface, xdg_wm_base_interface.name)) {
        xdg_wm_base = static_cast<decltype(xdg_wm_base)>(
            wl_registry_bind(wl_registry, name, &xdg_wm_base_interface, 1));
        xdg_wm_base_add_listener(xdg_wm_base, &kXdgWmBaseListener, this);
    } else if (!strcmp(interface, wl_shm_interface.name)) {
        shm = static_cast<decltype(shm)>(
            wl_registry_bind(wl_registry, name, &wl_shm_interface, 1));
    } else if (!strcmp(interface, wl_data_device_manager_interface.name)) {
        constexpr uint32_t kMaxVersion = 3;
        data_device_manager_version = std::min(kMaxVersion, version);
        data_device_manager = static_cast<decltype(data_device_manager)>(
            wl_registry_bind(wl_registry, name, &wl_data_device_manager_interface,
                            data_device_manager_version));
    } else if (!strcmp(interface, wl_output_interface.name)) {
        AddOutput(name);
    } else if (!strcmp(interface, wl_seat_interface.name))
        AddInput(name, version);
    }
}

} // namespace 
