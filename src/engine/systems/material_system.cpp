#include "material_component.h"
#include "material_system.h"
#include "named_resources.h"
#include "texture_component.h"

#include "texture_manager.h"

#include <typeindex>
#include <type_traits>

namespace ECS
{
    namespace SYSTEMS
    {
    	void FMaterialSystem::Init()
    	{
    		FBuffer MaterialsBuffer = RESOURCE_ALLOCATOR()->CreateBuffer(MAX_MATERIALS * sizeof(COMPONENTS::FOpenPBRSurfaceComponent),
					VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, MATERIAL_SYSTEM_DATA_BUFFER);
    		RESOURCE_ALLOCATOR()->RegisterBuffer(MaterialsBuffer, MATERIAL_SYSTEM_DATA_BUFFER);

    		for (int i = 0; i < MAX_MATERIALS; i++)
    		{
    			FreeIndices.push(i);
    		}
    	}

    	bool FMaterialSystem::Update()
    	{
    		/// TODO: Optimize loading
    		if (!ChangedMaterials.empty())
    		{
    			std::vector<COMPONENTS::FOpenPBRSurfaceComponent> UpdatedMaterials(ChangedMaterials.size());
    			std::vector<VkDeviceSize> Sizes(ChangedMaterials.size());
    			std::vector<VkDeviceSize> Offsets(ChangedMaterials.size());
    			std::vector<void*> DataPointers(ChangedMaterials.size());

    			int i = 0;

    			for (auto Entity : ChangedMaterials)
    			{
    				Sizes[i] = sizeof(COMPONENTS::FOpenPBRSurfaceComponent);
    				UpdatedMaterials[i] = COORDINATOR().GetComponent<COMPONENTS::FOpenPBRSurfaceComponent>(Entity);
    				Offsets[i] = sizeof(COMPONENTS::FOpenPBRSurfaceComponent) * MaterialToIndexMap[Entity];;
    				DataPointers[i] = &UpdatedMaterials[i];
    				++i;
    			}

    			RESOURCE_ALLOCATOR()->LoadDataToBuffer(MATERIAL_SYSTEM_DATA_BUFFER, Sizes, Offsets, DataPointers);
    			ChangedMaterials.clear();
    			return true;
    		}

    		return false;
    	}

        FEntity FMaterialSystem::CreateDefaultMaterial(const std::string& MaterialName)
        {
            FEntity Material = COORDINATOR().CreateEntity();
    		MaterialToName[Material] = MaterialName.empty() ? "Material entity " + std::to_string(Material) : MaterialName;
            COORDINATOR().AddComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(Material, {});
    		MaterialToIndexMap[Material] = FreeIndices.front();
    		FreeIndices.pop();
            return Material;
        }

		FEntity FMaterialSystem::CreateEmptyMaterial(const std::string& MaterialName)
		{
			FEntity Material = COORDINATOR().CreateEntity();
    		MaterialToName[Material] = MaterialName.empty() ? "Material entity " + std::to_string(Material) : MaterialName;
			COORDINATOR().AddComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(Material, {});
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(Material);

    		MaterialComponent.BaseWeight = 0;
    		MaterialComponent.BaseColor = {0.f, 0.f, 0.f};
    		MaterialComponent.BaseMetalness = 0;
    		MaterialComponent.BaseDiffuseRoughness = 0;
    		MaterialComponent.SpecularWeight = 0;
    		MaterialComponent.SpecularColor = {0.f, 0.f, 0.f};
    		MaterialComponent.SpecularRoughness = 0;
    		MaterialComponent.SpecularRoughnessAnisotropy = 0;
    		MaterialComponent.SpecularIor = 0;
    		MaterialComponent.TransmissionWeight = 0;
    		MaterialComponent.TransmissionColor = {0.f, 0.f, 0.f};
    		MaterialComponent.TransmissionDepth = 0;
    		MaterialComponent.TransmissionScatter = {0.f, 0.f, 0.f};
    		MaterialComponent.TransmissionScatterAnisotropy = 0;
    		MaterialComponent.TransmissionDispersionScale = 0;
    		MaterialComponent.TransmissionDispersionAbbeNumber = 0;
    		MaterialComponent.SubsurfaceWeight = 0;
    		MaterialComponent.SubsurfaceColor = {0.f, 0.f, 0.f};
    		MaterialComponent.SubsurfaceRadius = 0;
    		MaterialComponent.SubsurfaceRadiusScale = {0.f, 0.f, 0.f};
    		MaterialComponent.SubsurfaceScatterAnisotropy = 0;
    		MaterialComponent.CoatWeight = 0;
    		MaterialComponent.CoatColor = {0.f, 0.f, 0.f};
    		MaterialComponent.CoatRoughness = 0;
    		MaterialComponent.CoatRoughnessAnisotropy = 0;
    		MaterialComponent.CoatIor = 0;
    		MaterialComponent.CoatDarkening = 0;
    		MaterialComponent.FuzzWeight = 0;
    		MaterialComponent.FuzzColor = {0.f, 0.f, 0.f};
    		MaterialComponent.FuzzRoughness = 0;
    		MaterialComponent.EmissionLuminance = 0;
    		MaterialComponent.EmissionColor = {0.f, 0.f, 0.f};
    		MaterialComponent.ThinFilmWeight = 0;
    		MaterialComponent.ThinFilmThickness = 0;
    		MaterialComponent.ThinFilmIor = 0;
    		MaterialComponent.GeometryOpacity = 0;
    		MaterialComponent.GeometryThinWalled = false;
    		MaterialComponent.GeometryNormal = {0.f, 0.f, 0.f};
    		MaterialComponent.GeometryTangent = {0.f, 0.f, 0.f};
    		MaterialComponent.GeometryCoatNormal = {0.f, 0.f, 0.f};
    		MaterialComponent.GeometryCoatTangent = {0.f, 0.f, 0.f};

    		MaterialToIndexMap[Material] = FreeIndices.front();
    		FreeIndices.pop();

			return Material;
		}

    	FMaterialSystem& FMaterialSystem::SetF(FEntity MaterialEntity, uint32_t MaterialProperty, float Property)
		{
    		auto& MaterialComponent = GetComponent<COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
    		switch (MaterialProperty)
    		{
    		    // Base
    		    case(MATERIAL_BASE_WEIGHT):
    		    {
    		        MaterialComponent.BaseWeight = Property;
    		        break;
    		    }
    		    case(MATERIAL_BASE_METALNESS):
    		    {
    		        MaterialComponent.BaseMetalness = Property;
    		        break;
    		    }
    		    case(MATERIAL_BASE_DIFFUSE_ROUGHNESS):
    		    {
    		        MaterialComponent.BaseDiffuseRoughness = Property;
    		        break;
    		    }

    		    // Specular
    		    case(MATERIAL_SPECULAR_WEIGHT):
    		    {
    		        MaterialComponent.SpecularWeight = Property;
    		        break;
    		    }
    		    case(MATERIAL_SPECULAR_ROUGHNESS):
    		    {
    		        MaterialComponent.SpecularRoughness = Property;
    		        break;
    		    }
    		    case(MATERIAL_SPECULAR_ROUGHNESS_ANISOTROPY):
    		    {
    		        MaterialComponent.SpecularRoughnessAnisotropy = Property;
    		        break;
    		    }
    		    case(MATERIAL_SPECULAR_IOR):
    		    {
    		        MaterialComponent.SpecularIor = Property;
    		        break;
    		    }

    		    // Transmission
    		    case(MATERIAL_TRANSMISSION_WEIGHT):
    		    {
    		        MaterialComponent.TransmissionWeight = Property;
    		        break;
    		    }
    		    case(MATERIAL_TRANSMISSION_DEPTH):
    		    {
    		        MaterialComponent.TransmissionDepth = Property;
    		        break;
    		    }
    		    case(MATERIAL_TRANSMISSION_SCATTER_ANISOTROPY):
    		    {
    		        MaterialComponent.TransmissionScatterAnisotropy = Property;
    		        break;
    		    }
    		    case(MATERIAL_TRANSMISSION_DISPERSION_SCALE):
    		    {
    		        MaterialComponent.TransmissionDispersionScale = Property;
    		        break;
    		    }
    		    case(MATERIAL_TRANSMISSION_DISPERSION_ABBE_NUMBER):
    		    {
    		        MaterialComponent.TransmissionDispersionAbbeNumber = Property;
    		        break;
    		    }

    		    // Subsurface
    		    case(MATERIAL_SUBSURFACE_WEIGHT):
    		    {
    		        MaterialComponent.SubsurfaceWeight = Property;
    		        break;
    		    }
    		    case(MATERIAL_SUBSURFACE_RADIUS):
    		    {
    		        MaterialComponent.SubsurfaceRadius = Property;
    		        break;
    		    }
    		    case(MATERIAL_SUBSURFACE_SCATTER_ANISOTROPY):
    		    {
    		        MaterialComponent.SubsurfaceScatterAnisotropy = Property;
    		        break;
    		    }

    		    // Coat
    		    case(MATERIAL_COAT_WEIGHT):
    		    {
    		        MaterialComponent.CoatWeight = Property;
    		        break;
    		    }
    		    case(MATERIAL_COAT_ROUGHNESS):
    		    {
    		        MaterialComponent.CoatRoughness = Property;
    		        break;
    		    }
    		    case(MATERIAL_COAT_ROUGHNESS_ANISOTROPY):
    		    {
    		        MaterialComponent.CoatRoughnessAnisotropy = Property;
    		        break;
    		    }
    		    case(MATERIAL_COAT_IOR):
    		    {
    		        MaterialComponent.CoatIor = Property;
    		        break;
    		    }
    		    case(MATERIAL_COAT_DARKENING):
    		    {
    		        MaterialComponent.CoatDarkening = Property;
    		        break;
    		    }

    		    // Fuzz
    		    case(MATERIAL_FUZZ_WEIGHT):
    		    {
    		        MaterialComponent.FuzzWeight = Property;
    		        break;
    		    }
    		    case(MATERIAL_FUZZ_ROUGHNESS):
    		    {
    		        MaterialComponent.FuzzRoughness = Property;
    		        break;
    		    }

    		    // Emission
    		    case(MATERIAL_EMISSION_LUMINANCE):
    		    {
    		        MaterialComponent.EmissionLuminance = Property;
    		        break;
    		    }

    		    // Thin film
    		    case(MATERIAL_THIN_FILM_WEIGHT):
    		    {
    		        MaterialComponent.ThinFilmWeight = Property;
    		        break;
    		    }
    		    case(MATERIAL_THIN_FILM_THICKNESS):
    		    {
    		        MaterialComponent.ThinFilmThickness = Property;
    		        break;
    		    }
    		    case(MATERIAL_THIN_FILM_IOR):
    		    {
    		        MaterialComponent.ThinFilmIor = Property;
    		        break;
    		    }

    		    // Geometry
    		    case(MATERIAL_GEOMETRY_OPACITY):
    		    {
    		        MaterialComponent.GeometryOpacity = Property;
    		        break;
    		    }

    		    default:
    		        throw std::runtime_error("Invalid material property");
    		}
    		ChangedMaterials.insert(MaterialEntity);
    		return *this;
		}

    	FMaterialSystem& FMaterialSystem::Set3F(FEntity MaterialEntity, uint32_t MaterialProperty, FVector3 Property)
		{
		    auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
		    switch (MaterialProperty)
		    {
		        // Base
		        case(MATERIAL_BASE_COLOR):
		        {
		            MaterialComponent.BaseColor = Property;
		            break;
		        }

		        // Specular
		        case(MATERIAL_SPECULAR_COLOR):
		        {
		            MaterialComponent.SpecularColor = Property;
		            break;
		        }

		        // Transmission
		        case(MATERIAL_TRANSMISSION_COLOR):
		        {
		            MaterialComponent.TransmissionColor = Property;
		            break;
		        }
		        case(MATERIAL_TRANSMISSION_SCATTER):
		        {
		            MaterialComponent.TransmissionScatter = Property;
		            break;
		        }

		        // Subsurface
		        case(MATERIAL_SUBSURFACE_COLOR):
		        {
		            MaterialComponent.SubsurfaceColor = Property;
		            break;
		        }
		        case(MATERIAL_SUBSURFACE_RADIUS_SCALE):
		        {
		            MaterialComponent.SubsurfaceRadiusScale = Property;
		            break;
		        }

		        // Coat
		        case(MATERIAL_COAT_COLOR):
		        {
		            MaterialComponent.CoatColor = Property;
		            break;
		        }

		        // Fuzz
		        case(MATERIAL_FUZZ_COLOR):
		        {
		            MaterialComponent.FuzzColor = Property;
		            break;
		        }

		        // Emission
		        case(MATERIAL_EMISSION_COLOR):
		        {
		            MaterialComponent.EmissionColor = Property;
		            break;
		        }

		        // Geometry
		        case(MATERIAL_GEOMETRY_NORMAL):
		        {
		            MaterialComponent.GeometryNormal = Property;
		            break;
		        }
		        case(MATERIAL_GEOMETRY_TANGENT):
		        {
		            MaterialComponent.GeometryTangent = Property;
		            break;
		        }
		        case(MATERIAL_GEOMETRY_COAT_NORMAL):
		        {
		            MaterialComponent.GeometryCoatNormal = Property;
		            break;
		        }
		        case(MATERIAL_GEOMETRY_COAT_TANGENT):
		        {
		            MaterialComponent.GeometryCoatTangent = Property;
		            break;
		        }

		        default:
		            throw std::runtime_error("Invalid material property");
		    }
		    ChangedMaterials.insert(MaterialEntity);
		    return *this;
		}

    	FMaterialSystem& FMaterialSystem::SetTexture(FEntity MaterialEntity, uint32_t MaterialProperty, FEntity TextureEntity)
		{
		    auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
		    auto TextureIndex = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity).TextureIndex;

		    switch (MaterialProperty)
		    {
		        // Base
		        case(MATERIAL_BASE_COLOR):
		        {
		            MaterialComponent.BaseColorTextureIndex = TextureIndex;
		            break;
		        }
		        case(MATERIAL_BASE_WEIGHT):
		        {
		            MaterialComponent.BaseWeightTextureIndex = TextureIndex;
		            break;
		        }
		        case(MATERIAL_BASE_METALNESS):
		        {
		            MaterialComponent.BaseMetalnessTextureIndex = TextureIndex;
		            break;
		        }
		        case(MATERIAL_BASE_DIFFUSE_ROUGHNESS):
		        {
		            MaterialComponent.BaseDiffuseRoughnessTextureIndex = TextureIndex;
		            break;
		        }

		        // Specular
		        case(MATERIAL_SPECULAR_WEIGHT):
		        {
		            MaterialComponent.SpecularWeightTextureIndex = TextureIndex;
		            break;
		        }
		        case(MATERIAL_SPECULAR_COLOR):
		        {
		            MaterialComponent.SpecularColorTextureIndex = TextureIndex;
		            break;
		        }
		        case(MATERIAL_SPECULAR_ROUGHNESS):
		        {
		            MaterialComponent.SpecularRoughnessTextureIndex = TextureIndex;
		            break;
		        }
		        case(MATERIAL_SPECULAR_ROUGHNESS_ANISOTROPY):
		        {
		            MaterialComponent.SpecularRoughnessAnisotropyTextureIndex = TextureIndex;
		            break;
		        }
		        case(MATERIAL_SPECULAR_IOR):
		        {
		            MaterialComponent.SpecularIorTextureIndex = TextureIndex;
		            break;
		        }

		        // Transmission
		        case(MATERIAL_TRANSMISSION_WEIGHT):
		        {
		            MaterialComponent.TransmissionWeightTextureIndex = TextureIndex;
		            break;
		        }
		        case(MATERIAL_TRANSMISSION_COLOR):
		        {
		            MaterialComponent.TransmissionColorTextureIndex = TextureIndex;
		            break;
		        }
		        case(MATERIAL_TRANSMISSION_DEPTH):
		        {
		            MaterialComponent.TransmissionDepthTextureIndex = TextureIndex;
		            break;
		        }
		        case(MATERIAL_TRANSMISSION_SCATTER):
		        {
		            MaterialComponent.TransmissionScatterTextureIndex = TextureIndex;
		            break;
		        }
		        case(MATERIAL_TRANSMISSION_SCATTER_ANISOTROPY):
		        {
		            MaterialComponent.TransmissionScatterAnisotropyTextureIndex = TextureIndex;
		            break;
		        }
		        case(MATERIAL_TRANSMISSION_DISPERSION_SCALE):
		        {
		            MaterialComponent.TransmissionDispersionScaleTextureIndex = TextureIndex;
		            break;
		        }
		        case(MATERIAL_TRANSMISSION_DISPERSION_ABBE_NUMBER):
		        {
		            MaterialComponent.TransmissionDispersionAbbeNumberTextureIndex = TextureIndex;
		            break;
		        }

		        // Subsurface
		        case(MATERIAL_SUBSURFACE_WEIGHT):
		        {
		            MaterialComponent.SubsurfaceWeightTextureIndex = TextureIndex;
		            break;
		        }
		        case(MATERIAL_SUBSURFACE_COLOR):
		        {
		            MaterialComponent.SubsurfaceColorTextureIndex = TextureIndex;
		            break;
		        }
		        case(MATERIAL_SUBSURFACE_RADIUS):
		        {
		            MaterialComponent.SubsurfaceRadiusTextureIndex = TextureIndex;
		            break;
		        }
		        case(MATERIAL_SUBSURFACE_RADIUS_SCALE):
		        {
		            MaterialComponent.SubsurfaceRadiusScaleTextureIndex = TextureIndex;
		            break;
		        }
		        case(MATERIAL_SUBSURFACE_SCATTER_ANISOTROPY):
		        {
		            MaterialComponent.SubsurfaceScatterAnisotropyTextureIndex = TextureIndex;
		            break;
		        }

		        // Coat
		        case(MATERIAL_COAT_WEIGHT):
		        {
		            MaterialComponent.CoatWeightTextureIndex = TextureIndex;
		            break;
		        }
		        case(MATERIAL_COAT_COLOR):
		        {
		            MaterialComponent.CoatColorTextureIndex = TextureIndex;
		            break;
		        }
		        case(MATERIAL_COAT_ROUGHNESS):
		        {
		            MaterialComponent.CoatRoughnessTextureIndex = TextureIndex;
		            break;
		        }
		        case(MATERIAL_COAT_ROUGHNESS_ANISOTROPY):
		        {
		            MaterialComponent.CoatRoughnessAnisotropyTextureIndex = TextureIndex;
		            break;
		        }
		        case(MATERIAL_COAT_IOR):
		        {
		            MaterialComponent.CoatIorTextureIndex = TextureIndex;
		            break;
		        }
		        case(MATERIAL_COAT_DARKENING):
		        {
		            MaterialComponent.CoatDarkeningTextureIndex = TextureIndex;
		            break;
		        }

		        // Fuzz
		        case(MATERIAL_FUZZ_WEIGHT):
		        {
		            MaterialComponent.FuzzWeightTextureIndex = TextureIndex;
		            break;
		        }
		        case(MATERIAL_FUZZ_COLOR):
		        {
		            MaterialComponent.FuzzColorTextureIndex = TextureIndex;
		            break;
		        }
		        case(MATERIAL_FUZZ_ROUGHNESS):
		        {
		            MaterialComponent.FuzzRoughnessTextureIndex = TextureIndex;
		            break;
		        }

		        // Emission
		        case(MATERIAL_EMISSION_LUMINANCE):
		        {
		            MaterialComponent.EmissionLuminanceTextureIndex = TextureIndex;
		            break;
		        }
		        case(MATERIAL_EMISSION_COLOR):
		        {
		            MaterialComponent.EmissionColorTextureIndex = TextureIndex;
		            break;
		        }

		        // Thin film
		        case(MATERIAL_THIN_FILM_WEIGHT):
		        {
		            MaterialComponent.ThinFilmWeightTextureIndex = TextureIndex;
		            break;
		        }
		        case(MATERIAL_THIN_FILM_THICKNESS):
		        {
		            MaterialComponent.ThinFilmThicknessTextureIndex = TextureIndex;
		            break;
		        }
		        case(MATERIAL_THIN_FILM_IOR):
		        {
		            MaterialComponent.ThinFilmIorTextureIndex = TextureIndex;
		            break;
		        }

		        // Geometry
		        case(MATERIAL_GEOMETRY_OPACITY):
		        {
		            MaterialComponent.GeometryOpacityTextureIndex = TextureIndex;
		            break;
		        }
		        case(MATERIAL_GEOMETRY_THIN_WALLED):
		        {
		            MaterialComponent.GeometryThinWalledTextureIndex = TextureIndex;
		            break;
		        }
		        case(MATERIAL_GEOMETRY_NORMAL):
		        {
		            MaterialComponent.GeometryNormalTextureIndex = TextureIndex;
		            break;
		        }
		        case(MATERIAL_GEOMETRY_TANGENT):
		        {
		            MaterialComponent.GeometryTangentTextureIndex = TextureIndex;
		            break;
		        }
		        case(MATERIAL_GEOMETRY_COAT_NORMAL):
		        {
		            MaterialComponent.GeometryCoatNormalTextureIndex = TextureIndex;
		            break;
		        }
		        case(MATERIAL_GEOMETRY_COAT_TANGENT):
		        {
		            MaterialComponent.GeometryCoatTangentTextureIndex = TextureIndex;
		            break;
		        }

		        default:
		            throw std::runtime_error("Invalid material property");
		    }
		    ChangedMaterials.insert(MaterialEntity);
		    return *this;
		}

    	template <typename T> struct TypeToString;
    	template <> struct TypeToString<float> {static std::string Name() { return "SampleFloat"; }};
    	template <> struct TypeToString<uint32_t> {static std::string Name() { return "SampleUint"; }};
    	template <> struct TypeToString<int> {static std::string Name() { return "SampleInt"; }};
    	template <> struct TypeToString<FVector2> {static std::string Name() { return "SampleVec2"; }};
    	template <> struct TypeToString<FVector3> {static std::string Name() { return "SampleVec3"; }};
    	template <> struct TypeToString<FVector4> {static std::string Name() { return "SampleVec4"; }};

    	template <typename T>
    	std::string GenerateGetFunction(const std::string& ParameterName, uint32_t MaterialIndex, uint32_t DataOffset, uint32_t TextureOffset, uint32_t TextureIndex, bool bConvertToLinear = false)
    	{
    		assert((DataOffset % 4) == 0);
    		assert((TextureOffset % 4) == 0);
    		DataOffset = DataOffset / 4;
    		TextureOffset = TextureOffset / 4;
    		std::string Result = "    " + ParameterName;

    		if (UINT32_MAX == TextureIndex)
    		{
    			Result +=  " = " + TypeToString<T>::Name() + "(" + std::to_string(MaterialIndex) + ", " + std::to_string(DataOffset) + ");\r\n";
    		}
    		else
    		{
    			/// If we sample a texture, in some cases it might be in sRGB format, and we need to translate it to linear color space
    			if (bConvertToLinear)
    			{
    				Result += " = SRGBToLinear(" + TypeToString<T> ::Name()+ "(" + std::to_string(MaterialIndex) + ", " + std::to_string(TextureOffset) + ", TextureCoords));\r\n";
    			}
    			else
    			{
    				Result += " = " + TypeToString<T> ::Name()+ "(" + std::to_string(MaterialIndex) + ", " + std::to_string(TextureOffset) + ", TextureCoords);\r\n";
    			}
    		}

    		return Result;
    	}

		std::string FMaterialSystem::GenerateMaterialCode(FEntity MaterialEntity)
		{
		    std::string Result;

		    auto& MaterialComponent = GetComponent<COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
		    uint32_t MaterialIndex = MaterialToIndexMap[MaterialEntity];

		    Result += "FDeviceMaterial GetMaterial(vec2 TextureCoords)\r\n";
		    Result += "{\r\n";
		    Result += "    FDeviceMaterial Material;\r\n";

		    // Base
		    Result += GenerateGetFunction<float>(   "Material.BaseWeight",                          MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, BaseWeight),                          offsetof(COMPONENTS::FOpenPBRSurfaceComponent, BaseWeightTextureIndex),                          MaterialComponent.BaseWeightTextureIndex);
		    Result += GenerateGetFunction<FVector3>("Material.BaseColor",                           MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, BaseColor),                           offsetof(COMPONENTS::FOpenPBRSurfaceComponent, BaseColorTextureIndex),                           MaterialComponent.BaseColorTextureIndex, true);
		    Result += GenerateGetFunction<float>(   "Material.BaseMetalness",                       MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, BaseMetalness),                       offsetof(COMPONENTS::FOpenPBRSurfaceComponent, BaseMetalnessTextureIndex),                       MaterialComponent.BaseMetalnessTextureIndex);
		    Result += GenerateGetFunction<float>(   "Material.BaseDiffuseRoughness",                MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, BaseDiffuseRoughness),                offsetof(COMPONENTS::FOpenPBRSurfaceComponent, BaseDiffuseRoughnessTextureIndex),                MaterialComponent.BaseDiffuseRoughnessTextureIndex);

		    // Specular
		    Result += GenerateGetFunction<float>(   "Material.SpecularWeight",                      MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, SpecularWeight),                      offsetof(COMPONENTS::FOpenPBRSurfaceComponent, SpecularWeightTextureIndex),                      MaterialComponent.SpecularWeightTextureIndex);
		    Result += GenerateGetFunction<FVector3>("Material.SpecularColor",                       MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, SpecularColor),                       offsetof(COMPONENTS::FOpenPBRSurfaceComponent, SpecularColorTextureIndex),                       MaterialComponent.SpecularColorTextureIndex, true);
		    Result += GenerateGetFunction<float>(   "Material.SpecularRoughness",                   MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, SpecularRoughness),                   offsetof(COMPONENTS::FOpenPBRSurfaceComponent, SpecularRoughnessTextureIndex),                   MaterialComponent.SpecularRoughnessTextureIndex);
		    Result += GenerateGetFunction<float>(   "Material.SpecularRoughnessAnisotropy",         MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, SpecularRoughnessAnisotropy),         offsetof(COMPONENTS::FOpenPBRSurfaceComponent, SpecularRoughnessAnisotropyTextureIndex),         MaterialComponent.SpecularRoughnessAnisotropyTextureIndex);
		    Result += GenerateGetFunction<float>(   "Material.SpecularIor",                         MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, SpecularIor),                         offsetof(COMPONENTS::FOpenPBRSurfaceComponent, SpecularIorTextureIndex),                         MaterialComponent.SpecularIorTextureIndex);

		    // Transmission
		    Result += GenerateGetFunction<float>(   "Material.TransmissionWeight",                  MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, TransmissionWeight),                  offsetof(COMPONENTS::FOpenPBRSurfaceComponent, TransmissionWeightTextureIndex),                  MaterialComponent.TransmissionWeightTextureIndex);
		    Result += GenerateGetFunction<FVector3>("Material.TransmissionColor",                   MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, TransmissionColor),                   offsetof(COMPONENTS::FOpenPBRSurfaceComponent, TransmissionColorTextureIndex),                   MaterialComponent.TransmissionColorTextureIndex, true);
		    Result += GenerateGetFunction<float>(   "Material.TransmissionDepth",                   MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, TransmissionDepth),                   offsetof(COMPONENTS::FOpenPBRSurfaceComponent, TransmissionDepthTextureIndex),                   MaterialComponent.TransmissionDepthTextureIndex);
		    Result += GenerateGetFunction<FVector3>("Material.TransmissionScatter",                 MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, TransmissionScatter),                 offsetof(COMPONENTS::FOpenPBRSurfaceComponent, TransmissionScatterTextureIndex),                 MaterialComponent.TransmissionScatterTextureIndex);
		    Result += GenerateGetFunction<float>(   "Material.TransmissionScatterAnisotropy",       MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, TransmissionScatterAnisotropy),       offsetof(COMPONENTS::FOpenPBRSurfaceComponent, TransmissionScatterAnisotropyTextureIndex),       MaterialComponent.TransmissionScatterAnisotropyTextureIndex);
		    Result += GenerateGetFunction<float>(   "Material.TransmissionDispersionScale",         MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, TransmissionDispersionScale),         offsetof(COMPONENTS::FOpenPBRSurfaceComponent, TransmissionDispersionScaleTextureIndex),         MaterialComponent.TransmissionDispersionScaleTextureIndex);
		    Result += GenerateGetFunction<float>(   "Material.TransmissionDispersionAbbeNumber",    MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, TransmissionDispersionAbbeNumber),    offsetof(COMPONENTS::FOpenPBRSurfaceComponent, TransmissionDispersionAbbeNumberTextureIndex),    MaterialComponent.TransmissionDispersionAbbeNumberTextureIndex);

		    // Subsurface
		    Result += GenerateGetFunction<float>(   "Material.SubsurfaceWeight",                    MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, SubsurfaceWeight),                    offsetof(COMPONENTS::FOpenPBRSurfaceComponent, SubsurfaceWeightTextureIndex),                    MaterialComponent.SubsurfaceWeightTextureIndex);
		    Result += GenerateGetFunction<FVector3>("Material.SubsurfaceColor",                     MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, SubsurfaceColor),                     offsetof(COMPONENTS::FOpenPBRSurfaceComponent, SubsurfaceColorTextureIndex),                     MaterialComponent.SubsurfaceColorTextureIndex, true);
		    Result += GenerateGetFunction<float>(   "Material.SubsurfaceRadius",                    MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, SubsurfaceRadius),                    offsetof(COMPONENTS::FOpenPBRSurfaceComponent, SubsurfaceRadiusTextureIndex),                    MaterialComponent.SubsurfaceRadiusTextureIndex);
		    Result += GenerateGetFunction<FVector3>("Material.SubsurfaceRadiusScale",               MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, SubsurfaceRadiusScale),               offsetof(COMPONENTS::FOpenPBRSurfaceComponent, SubsurfaceRadiusScaleTextureIndex),               MaterialComponent.SubsurfaceRadiusScaleTextureIndex);
		    Result += GenerateGetFunction<float>(   "Material.SubsurfaceScatterAnisotropy",         MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, SubsurfaceScatterAnisotropy),         offsetof(COMPONENTS::FOpenPBRSurfaceComponent, SubsurfaceScatterAnisotropyTextureIndex),         MaterialComponent.SubsurfaceScatterAnisotropyTextureIndex);

		    // Coat
		    Result += GenerateGetFunction<float>(   "Material.CoatWeight",                          MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, CoatWeight),                          offsetof(COMPONENTS::FOpenPBRSurfaceComponent, CoatWeightTextureIndex),                          MaterialComponent.CoatWeightTextureIndex);
		    Result += GenerateGetFunction<FVector3>("Material.CoatColor",                           MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, CoatColor),                           offsetof(COMPONENTS::FOpenPBRSurfaceComponent, CoatColorTextureIndex),                           MaterialComponent.CoatColorTextureIndex, true);
		    Result += GenerateGetFunction<float>(   "Material.CoatRoughness",                       MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, CoatRoughness),                       offsetof(COMPONENTS::FOpenPBRSurfaceComponent, CoatRoughnessTextureIndex),                       MaterialComponent.CoatRoughnessTextureIndex);
		    Result += GenerateGetFunction<float>(   "Material.CoatRoughnessAnisotropy",             MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, CoatRoughnessAnisotropy),             offsetof(COMPONENTS::FOpenPBRSurfaceComponent, CoatRoughnessAnisotropyTextureIndex),             MaterialComponent.CoatRoughnessAnisotropyTextureIndex);
		    Result += GenerateGetFunction<float>(   "Material.CoatIor",                             MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, CoatIor),                             offsetof(COMPONENTS::FOpenPBRSurfaceComponent, CoatIorTextureIndex),                             MaterialComponent.CoatIorTextureIndex);
		    Result += GenerateGetFunction<float>(   "Material.CoatDarkening",                       MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, CoatDarkening),                       offsetof(COMPONENTS::FOpenPBRSurfaceComponent, CoatDarkeningTextureIndex),                       MaterialComponent.CoatDarkeningTextureIndex);

		    // Fuzz
		    Result += GenerateGetFunction<float>(   "Material.FuzzWeight",                          MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, FuzzWeight),                          offsetof(COMPONENTS::FOpenPBRSurfaceComponent, FuzzWeightTextureIndex),                          MaterialComponent.FuzzWeightTextureIndex);
		    Result += GenerateGetFunction<FVector3>("Material.FuzzColor",                           MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, FuzzColor),                           offsetof(COMPONENTS::FOpenPBRSurfaceComponent, FuzzColorTextureIndex),                           MaterialComponent.FuzzColorTextureIndex, true);
		    Result += GenerateGetFunction<float>(   "Material.FuzzRoughness",                       MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, FuzzRoughness),                       offsetof(COMPONENTS::FOpenPBRSurfaceComponent, FuzzRoughnessTextureIndex),                       MaterialComponent.FuzzRoughnessTextureIndex);

		    // Emission
		    Result += GenerateGetFunction<float>(   "Material.EmissionLuminance",                   MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, EmissionLuminance),                   offsetof(COMPONENTS::FOpenPBRSurfaceComponent, EmissionLuminanceTextureIndex),                   MaterialComponent.EmissionLuminanceTextureIndex);
		    Result += GenerateGetFunction<FVector3>("Material.EmissionColor",                       MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, EmissionColor),                       offsetof(COMPONENTS::FOpenPBRSurfaceComponent, EmissionColorTextureIndex),                       MaterialComponent.EmissionColorTextureIndex, true);

		    // Thin film
		    Result += GenerateGetFunction<float>(   "Material.ThinFilmWeight",                      MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, ThinFilmWeight),                      offsetof(COMPONENTS::FOpenPBRSurfaceComponent, ThinFilmWeightTextureIndex),                      MaterialComponent.ThinFilmWeightTextureIndex);
		    Result += GenerateGetFunction<float>(   "Material.ThinFilmThickness",                   MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, ThinFilmThickness),                   offsetof(COMPONENTS::FOpenPBRSurfaceComponent, ThinFilmThicknessTextureIndex),                   MaterialComponent.ThinFilmThicknessTextureIndex);
		    Result += GenerateGetFunction<float>(   "Material.ThinFilmIor",                         MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, ThinFilmIor),                         offsetof(COMPONENTS::FOpenPBRSurfaceComponent, ThinFilmIorTextureIndex),                         MaterialComponent.ThinFilmIorTextureIndex);

		    // Geometry
		    Result += GenerateGetFunction<float>(   "Material.GeometryOpacity",                     MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, GeometryOpacity),                     offsetof(COMPONENTS::FOpenPBRSurfaceComponent, GeometryOpacityTextureIndex),                     MaterialComponent.GeometryOpacityTextureIndex);
		    Result += GenerateGetFunction<uint32_t>("Material.GeometryThinWalled",                  MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, GeometryThinWalled),                  offsetof(COMPONENTS::FOpenPBRSurfaceComponent, GeometryThinWalledTextureIndex),                  MaterialComponent.GeometryThinWalledTextureIndex);
		    Result += GenerateGetFunction<FVector3>("Material.GeometryNormal",                      MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, GeometryNormal),                      offsetof(COMPONENTS::FOpenPBRSurfaceComponent, GeometryNormalTextureIndex),                      MaterialComponent.GeometryNormalTextureIndex);
		    Result += GenerateGetFunction<FVector3>("Material.GeometryTangent",                     MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, GeometryTangent),                     offsetof(COMPONENTS::FOpenPBRSurfaceComponent, GeometryTangentTextureIndex),                     MaterialComponent.GeometryTangentTextureIndex);
		    Result += GenerateGetFunction<FVector3>("Material.GeometryCoatNormal",                  MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, GeometryCoatNormal),                  offsetof(COMPONENTS::FOpenPBRSurfaceComponent, GeometryCoatNormalTextureIndex),                  MaterialComponent.GeometryCoatNormalTextureIndex);
		    Result += GenerateGetFunction<FVector3>("Material.GeometryCoatTangent",                 MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, GeometryCoatTangent),                 offsetof(COMPONENTS::FOpenPBRSurfaceComponent, GeometryCoatTangentTextureIndex),                 MaterialComponent.GeometryCoatTangentTextureIndex);

		    Result += "    return Material;\r\n";
		    Result += "};\r\n";

		    return Result;
		}

		std::string FMaterialSystem::GenerateEmissiveMaterialsCode(const std::unordered_map<uint32_t , uint32_t>& EmissiveMaterials)
		{
			std::string Result;

			if (EmissiveMaterials.empty())
			{
				Result += "FDeviceMaterial GetEmissiveMaterial(vec2 TextureCoords, uint MaterialIndex)\r\n";
				Result += "{\r\n";
				Result += "    FDeviceMaterial Material;\r\n";
				Result += "    return Material;\r\n";
				Result += "};\r\n";
				return Result;
			}

			Result += "FDeviceMaterial GetEmissiveMaterial(vec2 TextureCoords, uint MaterialIndex)\r\n";
			Result += "{\r\n";
			Result += "    FDeviceMaterial Material;\r\n";
			Result += "    switch(MaterialIndex)\r\n";
			Result += "    {\r\n";
			for (auto& Entry : EmissiveMaterials)
			{
				auto& MaterialComponent = GetComponentByIndex<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(Entry.first);
				Result += "    case " + std::to_string(Entry.first) + ":\r\n";
				Result += "    {\r\n";
				/// TODO: Fix the indexing
				Result += "    " + GenerateGetFunction<float>("Material.EmissionLuminance", 0, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, EmissionLuminance), offsetof(COMPONENTS::FOpenPBRSurfaceComponent, EmissionLuminanceTextureIndex), MaterialComponent.EmissionLuminanceTextureIndex);
				Result += "    " + GenerateGetFunction<FVector3>("Material.EmissionColor", 0, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, EmissionColor), offsetof(COMPONENTS::FOpenPBRSurfaceComponent, EmissionColorTextureIndex), MaterialComponent.EmissionColorTextureIndex);
				Result += "    break;\r\n";
				Result += "    }\r\n";
			}
			Result += "    };\r\n";

			Result += "    return Material;\r\n";
			Result += "};\r\n";

			return Result;
		}
    }
}
