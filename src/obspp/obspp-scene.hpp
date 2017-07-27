#include "obspp-input.hpp"

namespace obs {

class scene : public source {

private:
    obs_scene_t *m_scene;

public:
    class item {
        friend scene;

        obs_sceneitem_t *m_handle;
        obs::source::status_type m_status;

    public:
        item();
        item(obs_sceneitem_t *item);
        item(item &copy);

        obs::source::status_type status();

        void remove();
        obs::scene scene();
        obs::input source();
        uint64_t id();

        bool visible();
        void visible(bool is_visible);

        bool selected();
        void selected(bool select);

        void position(const vec2 &pos);
        vec2 position();

        void rotation(float rot);
        float rotation();

        void scale(const vec2 &scale);
        vec2 scale();

        void alignment(uint32_t alignment);
        uint32_t alignment();

        void bounds_alignment(uint32_t alignment);
        uint32_t bounds_alignment();

        void bounds(const vec2 &bounds);
        vec2 bounds();

        void transform_info(const obs_transform_info &info);
        obs_transform_info transform_info();

        void order(obs_order_movement movement);

        void order_position(int position);

        void bounds_type(obs_bounds_type type);
        obs_bounds_type bounds_type();

        void crop(const obs_sceneitem_crop &crop);
        obs_sceneitem_crop crop();

        void scale_filter(obs_scale_type filter);
        obs_scale_type scale_filter();

        /* When accessing tranform data (crop, tranform, etc.)
         * you need to do it between a begin and end call. Else
         * It's a data race. This acts as a basic barrier to 
         * the data from updating while you're accessing it. */
        void defer_update_begin();
        void defer_update_end();
    };

    scene(std::string name, bool is_private = false);
    scene(scene &copy);
    scene(obs_scene_t *source);
    scene(obs_source_t *source);

    static scene from_name(std::string name);

    bool operator!();
    obs_scene_t *dangerous_scene();
    obs::scene duplicate(std::string name, enum obs_scene_duplicate_type type);
    scene::item add(input source);
    obs::input source();
    obs::scene::item find_item(std::string name);
    obs::scene::item find_item(int64_t position);
    std::vector<scene::item> items();
};

}