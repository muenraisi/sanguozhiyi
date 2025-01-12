/*
 *  Copyright 2019-2021 Diligent Graphics LLC
 *  Copyright 2015-2019 Egor Yusov
 *  
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  
 *      http://www.apache.org/licenses/LICENSE-2.0
 *  
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  In no event and under no legal theory, whether in tort (including negligence), 
 *  contract, or otherwise, unless required by applicable law (such as deliberate 
 *  and grossly negligent acts) or agreed to in writing, shall any Contributor be
 *  liable for any damages, including any direct, indirect, special, incidental, 
 *  or consequential damages of any character arising as a result of this License or 
 *  out of the use or inability to use the software (including but not limited to damages 
 *  for loss of goodwill, work stoppage, computer failure or malfunction, or any and 
 *  all other commercial damages or losses), even if such Contributor has been advised 
 *  of the possibility of such damages.
 */

// This file is derived from the open source project provided by Intel Corportaion that
// requires the following notice to be kept:
//--------------------------------------------------------------------------------------
// Copyright 2013 Intel Corporation
// All Rights Reserved
//
// Permission is granted to use, copy, distribute and prepare derivative works of this
// software for any purpose and without fee, provided, that the above copyright notice
// and this statement appear in all copies.  Intel makes no representations about the
// suitability of this software for any purpose.  THIS SOFTWARE IS PROVIDED "AS IS."
// INTEL SPECIFICALLY DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED, AND ALL LIABILITY,
// INCLUDING CONSEQUENTIAL AND OTHER INDIRECT DAMAGES, FOR THE USE OF THIS SOFTWARE,
// INCLUDING LIABILITY FOR INFRINGEMENT OF ANY PROPRIETARY RIGHTS, AND INCLUDING THE
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  Intel does not
// assume any responsibility for any errors which may appear in this software nor any
// responsibility to update it.
//--------------------------------------------------------------------------------------

#pragma once

#include <vector>

#include "Buffer.h"
#include "BufferView.h"
#include "DeviceContext.h"
#include "GraphicsTypes.h"
#include "RefCntAutoPtr.hpp"
#include "RenderDevice.h"
#include "Texture.h"
#include "TextureView.h"

#include "AdvancedMath.hpp"

namespace Diligent
{

// Include structures in Diligent namespace
#include "../../assets/shaders/HostSharedTerrainStructs.fxh"

// Structure describing terrain rendering parameters
struct RenderingParams
{
  TerrainAttribs terrain_attribs;

  enum TEXTURING_MODE
  {
    TM_HEIGHT_BASED     = 0,
    TM_MATERIAL_MASK    = 1,
    TM_MATERIAL_MASK_NM = 2
  };

  // Patch rendering params
  TEXTURING_MODE texturing_mode                = TM_MATERIAL_MASK_NM;
  int            ring_dim                      = 65; // 4K+1
  int            num_rings                     = 15;
  int            num_shadow_cascades           = 6;
  int            best_cascade_search           = 1;
  int            fixed_shadow_filter_size      = 5;
  bool           filter_across_shadow_cascades = true;
  int            m_iColOffset                  = 1356;
  int            m_iRowOffset                  = 924;
  TEXTURE_FORMAT DstRTVFormat                  = TEX_FORMAT_R11G11B10_FLOAT;
  TEXTURE_FORMAT ShadowMapFormat               = TEX_FORMAT_D32_FLOAT;
};

struct RingSectorMesh
{
  RefCntAutoPtr<IBuffer> index_buffer;
  size_t                 num_indices;
  BoundBox               bound_box;
  RingSectorMesh() : num_indices(0) {}
};

class EarthHemsiphere
{
public:
  EarthHemsiphere(void) : m_ValidShaders(0) {}

  // clang-format off
  // 显式的禁用某个函数
  EarthHemsiphere             (const EarthHemsiphere&) = delete; //默认复制构造禁用
  EarthHemsiphere& operator = (const EarthHemsiphere&) = delete; //默认赋值构造禁用
  EarthHemsiphere             (EarthHemsiphere&&)      = delete; //指针复制构造禁用
  EarthHemsiphere& operator = (EarthHemsiphere&&)      = delete; //指针赋值构造禁用
  // clang-format on

  // Renders the model

  void Render(IDeviceContext*        pContext,
              const RenderingParams& NewParams,
              const float4x4&        CameraViewProjMatrix,
              ITextureView*          pShadowMapSRV,
              ITextureView*          pPrecomputedNetDensitySRV,
              ITextureView*          pAmbientSkylightSRV,
              bool                   bZOnlyPass);

  // Creates device resources
  void Create(class ElevationDataSource* pDataSource,
              const RenderingParams&     Params,
              IRenderDevice*             pDevice,
              IDeviceContext*            pContext,
              const Char*                MaterialMaskPath,
              const Char*                TileTexturePath[],
              const Char*                TileNormalMapPath[],
              IBuffer*                   pcbCameraAttribs,
              IBuffer*                   pcbLightAttribs,
              IBuffer*                   pcMediaScatteringParams);

  enum
  {
    NUM_TILE_TEXTURES = 1 + 4
  }; // One base material + 4 masked materials

private:
  void RenderNormalMap(IRenderDevice*  pd3dDevice,
                       IDeviceContext* pd3dImmediateContext,
                       const Uint16*   pHeightMap,
                       size_t          HeightMapPitch,
                       size_t          HeightMapDim,
                       ITexture*       ptex2DNormalMap);

  RenderingParams params_;

  RefCntAutoPtr<IRenderDevice> device_;

  RefCntAutoPtr<IBuffer>      terrain_attribs_buffer_;
  RefCntAutoPtr<IBuffer>      vertex_buffer_;
  RefCntAutoPtr<ITextureView> normal_map_srv, m_ptex2DMtrlMaskSRV;

  RefCntAutoPtr<ITextureView> m_ptex2DTilesSRV[NUM_TILE_TEXTURES];
  RefCntAutoPtr<ITextureView> m_ptex2DTilNormalMapsSRV[NUM_TILE_TEXTURES];

  RefCntAutoPtr<IResourceMapping> resource_mapping_;
  RefCntAutoPtr<IShader>          hemisphere_vs_;

  RefCntAutoPtr<IPipelineState>         hemisphere_z_only_pso_;
  RefCntAutoPtr<IShaderResourceBinding> hemisphere_z_only_srb_;
  RefCntAutoPtr<IPipelineState>         hemisphere_pso_;
  RefCntAutoPtr<IShaderResourceBinding> hemisphere_srb_;
  RefCntAutoPtr<ISampler>               comparison_sampler_;

  std::vector<RingSectorMesh> sphere_meshes_;

  Uint32 m_ValidShaders;
};

} // namespace Diligent
