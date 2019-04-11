// #ifndef RENDERING_H
// #define RENDERING_H

// #include "engine.h"
// #include "renderer.h"

// // struct SpriteData {
// //     int16_t x, y;
// //     int w, h;
// //     SDL_Color color;
// //     int sprite_index;
// //     std::string sprite_name;
// //     float rotation;
// //     int layer;

// //     bool operator<(const SpriteData &rhs) const { 
// //         return layer < rhs.layer; 
// //     }
// // };

// struct SpriteData {
//     SDL_Texture *tex;
//     SDL_Rect src;
//     SDL_Rect dest;
//     float angle;
//     int layer;

//     bool operator<(const SpriteData &rhs) const { 
//         return layer < rhs.layer; 
//     }
// };

// static std::vector<SpriteSheet> *sprite_sheets;

// struct RenderBuffer {    
//     int sprite_count = 0;
//     SpriteData *sprite_data_buffer;
    
//     void init(size_t sz) {
//         sprite_data_buffer = new SpriteData[sz];
//     }

//     void prepare() {
//         clear();
//         sprite_sheets = &Resources::get_sprite_sheets();
//     }

//     void clear() {
//         sprite_count = 0;
//     }
// };

// // template<typename T>
// // void export_sprite_data(const T &entity_data, const int i, SpriteData &spr) {
// //     // handle camera, zoom and stuff here

// //     // also we can do culling here
// //     // intersects world_bounds etc

// //     // float globalScale = 0.05f;
// //     // spr.x = go.pos.x * globalScale;
// //     // spr.y = go.pos.y * globalScale;
// //     // spr.scale = go.sprite.scale * globalScale;
// //     // spr.x = entity_data.position[i].x - camera.x;
// //     // spr.x = entity_data.position[i].y - camera.y;

// //     const auto &camera = get_camera();

// //     spr.x = (int16_t)(entity_data.position[i].value.x - camera.x);
// //     spr.y = (int16_t)(entity_data.position[i].value.y - camera.y);
// //     spr.w = entity_data.sprite[i].w;
// //     spr.h = entity_data.sprite[i].h;
// //     spr.sprite_index = entity_data.sprite[i].sprite_sheet_index;
// //     spr.sprite_name = entity_data.sprite[i].sprite_name;
// //     spr.rotation = entity_data.sprite[i].rotation;
// //     spr.layer = entity_data.sprite[i].layer;
// // }

// // template<typename T>
// // bool export_sprite_data_values_cull(const Vector2 &position, const T &sprite, const int i, SpriteData &spr) {
// //     // handle camera, zoom and stuff here

// //     // also we can do culling here
// //     // intersects world_bounds etc

// //     // float globalScale = 0.05f;
// //     // spr.x = go.pos.x * globalScale;
// //     // spr.y = go.pos.y * globalScale;
// //     // spr.scale = go.sprite.scale * globalScale;
// //     // spr.x = entity_data.position[i].x - camera.x;
// //     // spr.x = entity_data.position[i].y - camera.y;

// //     const auto &camera = get_camera();
// //     int16_t x = (int16_t)(position.x - camera.x);
// //     int16_t y = (int16_t)(position.y - camera.y);
// //     int w = sprite.w;
// //     int h = sprite.h;

// //     // Camera view is always the same since we switch 
// //     // sprites position to render them outside view
// //     int sprite_half_size = 8;
// //     Rectangle camera_view; 
// //     camera_view.x = 0;
// //     camera_view.y = 0;
// //     camera_view.w = gw + sprite_half_size;
// //     camera_view.h = gh + sprite_half_size;

// //     Rectangle item;
// //     item.x = x;
// //     item.y = y;
// //     item.w = w;
// //     item.h = h;

// //     if(!Math::intersect_AABB(camera_view, item)) {
// //         return false;
// //     }
    
// //     spr.x = x;
// //     spr.y = y;
// //     spr.w = w;
// //     spr.h = h;
// //     spr.sprite_index = sprite.sprite_sheet_index;
// //     spr.sprite_name = sprite.sprite_name;
// //     spr.rotation = sprite.rotation;
// //     spr.layer = sprite.layer;

// //     return true;
// // }

// // draw_sprite_region_centered_ex(Resources::sprite_get(s.sprite_sheet_name), 
// // &s.sheet_sprites[s.sprites_by_name.at(sprite)].region, x, y, w, h, angle);


// template<typename T>
// void export_sprite_data(const T &entity_data, const int i, SpriteData &spr) {
//     // handle camera, zoom and stuff here

//     // also we can do culling here
//     // intersects world_bounds etc

//     // float globalScale = 0.05f;
//     // spr.x = go.pos.x * globalScale;
//     // spr.y = go.pos.y * globalScale;
//     // spr.scale = go.sprite.scale * globalScale;
//     // spr.x = entity_data.position[i].x - camera.x;
//     // spr.x = entity_data.position[i].y - camera.y;

//     const auto &camera = get_camera();

//     auto &sheet = sprite_sheets->at(entity_data.sprite[i].sprite_sheet_index);
//     auto &region = sheet.sheet_sprites[sheet.sprites_by_name.at(entity_data.sprite[i].sprite_name)].region;

//     spr.tex = Resources::sprite_get(sheet.sprite_sheet_name)->image;
//     spr.src = region;

//     spr.angle = entity_data.sprite[i].rotation;
//     spr.layer = entity_data.sprite[i].layer;

//     if(entity_data.sprite[i].line) {
//         spr.dest.x = (int16_t)(entity_data.sprite[i].position.x - camera.x);
//         spr.dest.y = (int16_t)(entity_data.sprite[i].position.y - camera.y);
//         spr.dest.w = entity_data.sprite[i].w;
//         spr.dest.h = entity_data.sprite[i].h;
//     } else {
//         spr.dest.x = (int16_t)(entity_data.position[i].value.x - camera.x);
//         spr.dest.y = (int16_t)(entity_data.position[i].value.y - camera.y);

//         spr.dest.w = entity_data.sprite[i].w;
//         spr.dest.h = entity_data.sprite[i].h;

//         spr.dest.x = spr.dest.x - (spr.dest.w / 2);
//         spr.dest.y = spr.dest.y - (spr.dest.h / 2);
//     }
// }

// template<typename T>
// bool export_sprite_data_values_cull(const Vector2 &position, const T &sprite, const int i, SpriteData &spr) {
//     // handle camera, zoom and stuff here

//     // also we can do culling here
//     // intersects world_bounds etc

//     // float globalScale = 0.05f;
//     // spr.x = go.pos.x * globalScale;
//     // spr.y = go.pos.y * globalScale;
//     // spr.scale = go.sprite.scale * globalScale;
//     // spr.x = entity_data.position[i].x - camera.x;
//     // spr.x = entity_data.position[i].y - camera.y;

//     const auto &camera = get_camera();
//     int16_t x = (int16_t)(position.x - camera.x);
//     int16_t y = (int16_t)(position.y - camera.y);
//     int w = sprite.w;
//     int h = sprite.h;

//     // Camera view is always the same since we switch 
//     // sprites position to render them outside view
//     int sprite_half_size = 8;
//     Rectangle camera_view; 
//     camera_view.x = 0;
//     camera_view.y = 0;
//     camera_view.w = gw + sprite_half_size;
//     camera_view.h = gh + sprite_half_size;

//     Rectangle item;
//     item.x = x;
//     item.y = y;
//     item.w = w;
//     item.h = h;

//     if(!Math::intersect_AABB(camera_view, item)) {
//         return false;
//     }
    
//     auto &sheet = sprite_sheets->at(sprite.sprite_sheet_index);
//     auto *region = &sheet.sheet_sprites[sheet.sprites_by_name.at(sprite.sprite_name)].region;

//     spr.dest.x = x;
//     spr.dest.y = y;
//     spr.dest.w = w;
//     spr.dest.h = h;
    
//     spr.dest.x = x - (w / 2);
//     spr.dest.y = y - (h / 2);
    
//     spr.tex = Resources::sprite_get(sheet.sprite_sheet_name)->image;
//     spr.src = region;
    
//     spr.angle = sprite.rotation;
//     spr.layer = sprite.layer;

//     return true;
// }

// void draw_buffer(const SpriteData *spr, const int length) {
//     for(int i = 0; i < length; i++) {    
//         SDL_RenderCopyEx(renderer.renderer, spr[i].tex, &spr[i].src, &spr[i].dest, spr[i].angle, NULL, SDL_FLIP_NONE);
//     }
// }

// void export_background(RenderBuffer &render_buffer, GameArea *_g) {
//     int tile_size = 16;
//     // Remove one so we dont draw to end of border
//     int w = (_g->world_bounds.w / tile_size) - 1;
//     int h = (_g->world_bounds.h / tile_size) - 1;
//     auto camera = get_camera();
//     int start_x = Math::max_i(0, (int)camera.x / tile_size);
//     int start_y = Math::max_i(0, (int)camera.y / tile_size);
    
//     auto index = Resources::sprite_sheet_index("deserts");
//     auto s = sprite_sheets->at(index);
    
//     int end_x = Math::min_i(start_x + (gw / 16) + 1, w);
//     int end_y = Math::min_i(start_y + (gh / 16) + 1, h);
    
//     auto sprite_data_buffer = render_buffer.sprite_data_buffer;
//     auto &sprite_count = render_buffer.sprite_count;

//     for(int x = start_x; x <= end_x; x++) {
//         for(int y = start_y; y <= end_y; y++) {
//             SpriteData &spr = sprite_data_buffer[sprite_count++];
//             spr.dest.x = x * tile_size - (int)camera.x;
//             spr.dest.y = y * tile_size - (int)camera.y;
//             spr.dest.w = tile_size;
//             spr.dest.h = tile_size;
            
//             spr.tex = Resources::sprite_get(s.sprite_sheet_name)->image;

//             spr.angle = 0;
//             spr.layer = 0;

//             float n = Noise::perlin((float)x, (float)y);
//             if(n < 0) {
//                 n = -n;
//             }

//             if(n <= 0.1f) {
//                 spr.src = s.sheet_sprites[0].region;
//             } else if(n <= 0.2f) {
//                 spr.src = s.sheet_sprites[1].region;
//             } else if(n <= 0.3f) {
//                 spr.src = s.sheet_sprites[2].region;
//             } else if(n <= 0.4f) {
//                 spr.src = s.sheet_sprites[3].region;
//             } else if(n <= 0.5f) {
//                 spr.src = s.sheet_sprites[4].region;
//             } else if(n <= 0.6f) {
//                 spr.src = s.sheet_sprites[5].region;
//             } else if(n <= 0.7f) {
//                 spr.src = s.sheet_sprites[6].region;
//             } else if(n <= 0.8f) {
//                 spr.src = s.sheet_sprites[7].region;
//             } else if(n <= 0.9f) {
//                 spr.src = s.sheet_sprites[8].region;
//             } else {
//                 spr.src = s.sheet_sprites[9].region;
//             }
//         }
//     }
// }

// void export_render_info(RenderBuffer &render_buffer, GameArea *_g) {
//     render_buffer.prepare();
//     auto sprite_data_buffer = render_buffer.sprite_data_buffer;
//     auto &sprite_count = render_buffer.sprite_count;

//     export_background(render_buffer, _g);

//     /*
//     auto &tiles = _g->tiles;
//     for(size_t i = 0; i < tiles.size(); i++) {
//         if(export_sprite_data_values_cull(tiles[i].position, tiles[i].sprite, i, sprite_data_buffer[sprite_count])) {
//             sprite_count++;
//         }
//     }*/

//     for(int i = 0; i < _g->players.length; i++) {
//         Direction &d = _g->players.direction[i];
//         _g->players.sprite[i].rotation = d.angle + 90; // sprite is facing upwards so we need to adjust
//         export_sprite_data(_g->players, i, sprite_data_buffer[sprite_count++]);
//     }

//     for(size_t i = 0; i < _g->players.child_sprites.length; ++i) {
//         export_sprite_data(_g->players.child_sprites, i, sprite_data_buffer[sprite_count++]);
//         // export_sprite_data_values(players.child_sprites.position[i], players.child_sprites[i].sprite, i, sprite_data_buffer[sprite_count++]);
//     }

//     for(int i = 0; i < _g->projectiles_player.length; ++i) {
//         // Direction &d = _g->projectiles_player.direction[i];
//         // _g->projectiles_player.sprite[i].rotation = d.angle + 90;
//         export_sprite_data(_g->projectiles_player, i, sprite_data_buffer[sprite_count++]);
// 	}

//     for(int i = 0; i < _g->projectiles_target.length; ++i) {
//         // Direction &d = _g->projectiles_target.direction[i];
//         // _g->projectiles_target.sprite[i].rotation = d.angle + 90;
//         export_sprite_data(_g->projectiles_target, i, sprite_data_buffer[sprite_count++]);
// 	}

//     for(int i = 0; i < _g->targets.length; ++i) {
//         Direction &d = _g->targets.direction[i];
//         _g->targets.sprite[i].rotation = d.angle + 90;
//         export_sprite_data(_g->targets, i, sprite_data_buffer[sprite_count++]);
// 	}
    
//     for(size_t i = 0; i < _g->targets.child_sprites.length; ++i) {
//         export_sprite_data(_g->targets.child_sprites, i, sprite_data_buffer[sprite_count++]);
//         // export_sprite_data_values(players.child_sprites.position[i], players.child_sprites[i].sprite, i, sprite_data_buffer[sprite_count++]);
//     }

//     for(int i = 0; i < _g->effects.length; ++i) {
//         export_sprite_data(_g->effects, i, sprite_data_buffer[sprite_count++]);
// 	}

//     for(int i = 0; i < _g->drops.length; ++i) {
//         export_sprite_data(_g->drops, i, sprite_data_buffer[sprite_count++]);
// 	}

//     // Sort the render buffer by layer
//     std::sort(sprite_data_buffer, sprite_data_buffer + sprite_count);
// }
// #endif