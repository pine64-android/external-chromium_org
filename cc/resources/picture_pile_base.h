// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CC_RESOURCES_PICTURE_PILE_BASE_H_
#define CC_RESOURCES_PICTURE_PILE_BASE_H_

#include <bitset>
#include <list>
#include <utility>

#include "base/containers/hash_tables.h"
#include "base/memory/ref_counted.h"
#include "cc/base/cc_export.h"
#include "cc/base/region.h"
#include "cc/base/tiling_data.h"
#include "cc/resources/picture.h"
#include "ui/gfx/size.h"

namespace base {
namespace debug {
class TracedValue;
}
class Value;
}

namespace cc {

class CC_EXPORT PicturePileBase : public base::RefCounted<PicturePileBase> {
 public:
  PicturePileBase();
  explicit PicturePileBase(const PicturePileBase* other);

  gfx::Size tiling_size() const { return tiling_.tiling_size(); }
  void SetMinContentsScale(float min_contents_scale);

  // If non-empty, all pictures tiles inside this rect are recorded. There may
  // be recordings outside this rect, but everything inside the rect is
  // recorded.
  gfx::Rect recorded_viewport() const { return recorded_viewport_; }

  int num_tiles_x() const { return tiling_.num_tiles_x(); }
  int num_tiles_y() const { return tiling_.num_tiles_y(); }
  gfx::Rect tile_bounds(int x, int y) const { return tiling_.TileBounds(x, y); }
  bool HasRecordingAt(int x, int y);
  bool CanRaster(float contents_scale, const gfx::Rect& content_rect);

  // If this pile contains any valid recordings. May have false positives.
  bool HasRecordings() const { return has_any_recordings_; }

  // If this pile has ever contained any recordings with text.
  bool has_text() const { return has_text_; }

  bool is_solid_color() const { return is_solid_color_; }
  SkColor solid_color() const { return solid_color_; }

  void set_is_mask(bool is_mask) { is_mask_ = is_mask; }
  bool is_mask() const { return is_mask_; }

  static void ComputeTileGridInfo(const gfx::Size& tile_grid_size,
                                  SkTileGridFactory::TileGridInfo* info);

  void SetTileGridSize(const gfx::Size& tile_grid_size);
  TilingData& tiling() { return tiling_; }

  void AsValueInto(base::debug::TracedValue* array) const;

 protected:
  class CC_EXPORT PictureInfo {
   public:
    enum {
      INVALIDATION_FRAMES_TRACKED = 32
    };

    PictureInfo();
    ~PictureInfo();

    bool Invalidate(int frame_number);
    bool NeedsRecording(int frame_number, int distance_to_visible);
    void SetPicture(scoped_refptr<Picture> picture);
    const Picture* GetPicture() const;

    float GetInvalidationFrequencyForTesting() const {
      return GetInvalidationFrequency();
    }

   private:
    void AdvanceInvalidationHistory(int frame_number);
    float GetInvalidationFrequency() const;

    int last_frame_number_;
    scoped_refptr<const Picture> picture_;
    std::bitset<INVALIDATION_FRAMES_TRACKED> invalidation_history_;
  };

  typedef std::pair<int, int> PictureMapKey;
  typedef base::hash_map<PictureMapKey, PictureInfo> PictureMap;

  virtual ~PicturePileBase();

  int buffer_pixels() const { return tiling_.border_texels(); }
  void Clear();

  gfx::Rect PaddedRect(const PictureMapKey& key) const;
  gfx::Rect PadRect(const gfx::Rect& rect) const;

  // An internal CanRaster check that goes to the picture_map rather than
  // using the recorded_viewport hint.
  bool CanRasterSlowTileCheck(const gfx::Rect& layer_rect) const;

  // A picture pile is a tiled set of pictures. The picture map is a map of tile
  // indices to picture infos.
  PictureMap picture_map_;
  TilingData tiling_;
  gfx::Rect recorded_viewport_;
  float min_contents_scale_;
  SkTileGridFactory::TileGridInfo tile_grid_info_;
  SkColor background_color_;
  int slow_down_raster_scale_factor_for_debug_;
  bool contents_opaque_;
  bool contents_fill_bounds_completely_;
  bool show_debug_picture_borders_;
  bool clear_canvas_with_debug_color_;
  // A hint about whether there are any recordings. This may be a false
  // positive.
  bool has_any_recordings_;
  bool has_text_;
  bool is_mask_;
  bool is_solid_color_;
  SkColor solid_color_;

 private:
  void SetBufferPixels(int buffer_pixels);

  friend class base::RefCounted<PicturePileBase>;
  DISALLOW_COPY_AND_ASSIGN(PicturePileBase);
};

}  // namespace cc

#endif  // CC_RESOURCES_PICTURE_PILE_BASE_H_
