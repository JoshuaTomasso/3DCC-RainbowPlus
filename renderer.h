// minimalistic code to draw a single triangle, this is not part of the API.
#include "shaderc/shaderc.h" // needed for compiling shaders at runtime
#include <chrono>
#include <cstdlib>
#include <ctime>
#ifdef _WIN32 // must use MT platform DLL libraries on windows
	#pragma comment(lib, "shaderc_combined.lib") 
#endif

void PrintLabeledDebugString(const char* label, const char* toPrint)
{
	std::cout << label << toPrint << std::endl;
#if defined WIN32 //OutputDebugStringA is a windows-only function 
	OutputDebugStringA(label);
	OutputDebugStringA(toPrint);
#endif
}

// TODO: Part 4b
// TODO: Part 4c (inside new shader)
// TODO: Part 4d (inside new shader)
// TODO: Part 4e (inside new shader)

// Creation, Rendering & Cleanup
class Renderer
{
	// proxy handles
	GW::SYSTEM::GWindow win;
	GW::GRAPHICS::GVulkanSurface vlk;
	VkRenderPass renderPass;
	GW::CORE::GEventReceiver shutdown;

	// what we need at a minimum to draw a triangle
	VkDevice device = nullptr;
	VkPhysicalDevice physicalDevice = nullptr;
	VkBuffer vertexHandle = nullptr;
	VkDeviceMemory vertexData = nullptr;
	VkShaderModule vertexShader = nullptr;
	VkShaderModule fragmentShader = nullptr;

	// pipeline settings for drawing (also required)
	VkPipeline pipeline = nullptr;
	VkPipelineLayout pipelineLayout = nullptr;

	// TODO: Part 2a
	GW::MATH::GMATRIXF rotationMatrix;
	GW::MATH::GMatrix mMatrix;
	// TODO: Part 2b
	std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
	// TODO: Part 3a
	VkPipeline trianglePipeline = nullptr;
	// TODO: Part 3c
	VkBuffer indexHandle = nullptr;
	VkDeviceMemory indexData = nullptr;
	uint32_t indexCount = 0; // number of indices in the index buffer
	// TODO: Part 4a
	// TODO: Part 4b
	VkShaderModule outlineFragmentShader = nullptr;

	unsigned int windowWidth, windowHeight;
public:
	Renderer(GW::SYSTEM::GWindow _win, GW::GRAPHICS::GVulkanSurface _vlk)
	{
		win = _win;
		vlk = _vlk;
		mMatrix.Create(); // Create a matrix to transform the vertices
		UpdateWindowDimensions();
		InitializeGraphics();
		BindShutdownCallback();
	}

	private:
	void UpdateWindowDimensions()
	{
		win.GetClientWidth(windowWidth);
		win.GetClientHeight(windowHeight);
	}

	void InitializeGraphics()
	{
		// TODO: Part 2a
		GetHandlesFromSurface();
		InitializeVertexBuffer();
		// TODO: Part 3c 
		CompileShaders();
		InitializeGraphicsPipeline();
	}

	void GetHandlesFromSurface()
	{
		vlk.GetDevice((void**)&device);
		vlk.GetPhysicalDevice((void**)&physicalDevice);
		vlk.GetRenderPass((void**)&renderPass);
	}

	void InitializeVertexBuffer()
	{
		// TODO: Part 1b
		// TODO: Part 1c
		// TODO: Part 4a
		srand(static_cast<unsigned int>(time(nullptr)));
		Vertex verts[] = {
			// Position      // Random RGBA color
			{-0.25f,  0.75f, rand() / static_cast<float>(RAND_MAX), rand() / static_cast<float>(RAND_MAX), rand() / static_cast<float>(RAND_MAX), 1.0f}, // 1
			{ 0.25f,  0.75f, rand() / static_cast<float>(RAND_MAX), rand() / static_cast<float>(RAND_MAX), rand() / static_cast<float>(RAND_MAX), 1.0f}, // 2
			{ 0.25f,  0.25f, rand() / static_cast<float>(RAND_MAX), rand() / static_cast<float>(RAND_MAX), rand() / static_cast<float>(RAND_MAX), 1.0f}, // 3
			{ 0.75f,  0.25f, rand() / static_cast<float>(RAND_MAX), rand() / static_cast<float>(RAND_MAX), rand() / static_cast<float>(RAND_MAX), 1.0f}, // 4
			{ 0.75f, -0.25f, rand() / static_cast<float>(RAND_MAX), rand() / static_cast<float>(RAND_MAX), rand() / static_cast<float>(RAND_MAX), 1.0f}, // 5
			{ 0.25f, -0.25f, rand() / static_cast<float>(RAND_MAX), rand() / static_cast<float>(RAND_MAX), rand() / static_cast<float>(RAND_MAX), 1.0f}, // 6
			{ 0.25f, -0.75f, rand() / static_cast<float>(RAND_MAX), rand() / static_cast<float>(RAND_MAX), rand() / static_cast<float>(RAND_MAX), 1.0f}, // 7
			{-0.25f, -0.75f, rand() / static_cast<float>(RAND_MAX), rand() / static_cast<float>(RAND_MAX), rand() / static_cast<float>(RAND_MAX), 1.0f}, // 8
			{-0.25f, -0.25f, rand() / static_cast<float>(RAND_MAX), rand() / static_cast<float>(RAND_MAX), rand() / static_cast<float>(RAND_MAX), 1.0f}, // 9
			{-0.75f, -0.25f, rand() / static_cast<float>(RAND_MAX), rand() / static_cast<float>(RAND_MAX), rand() / static_cast<float>(RAND_MAX), 1.0f}, // 10
			{-0.75f,  0.25f, rand() / static_cast<float>(RAND_MAX), rand() / static_cast<float>(RAND_MAX), rand() / static_cast<float>(RAND_MAX), 1.0f}, // 11
			{-0.25f,  0.25f, rand() / static_cast<float>(RAND_MAX), rand() / static_cast<float>(RAND_MAX), rand() / static_cast<float>(RAND_MAX), 1.0f}, // 12
			{-0.25f,  0.75f, rand() / static_cast<float>(RAND_MAX), rand() / static_cast<float>(RAND_MAX), rand() / static_cast<float>(RAND_MAX), 1.0f}, // 13
		};
		CreateVertexBuffer(&verts[0], sizeof(verts));
		CreateIndexBuffer(); // Create the index buffer for the triangle
	}

	void CreateVertexBuffer(const void* data, unsigned int sizeInBytes)
	{
		// Transfer triangle data to the vertex buffer. (staging would be prefered here)
		GvkHelper::create_buffer(physicalDevice, device, sizeInBytes,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &vertexHandle, &vertexData);
		GvkHelper::write_to_buffer(device, vertexData, data, sizeInBytes);
	}

	void CreateIndexBuffer()
	{
		// These indices define triangles in clockwise order to cover the plus sign
		// Each set of 3 indices defines one triangle
		uint16_t indices[] = {
			// Top section (rectangular area)
			0, 1, 2,    // Triangle 1: top-left, top-right, right-top
			0, 2, 11,   // Triangle 2: top-left, right-top, left-top

			// Right section
			2, 3, 4,    // Triangle 3: right-top, right-right, right-bottom
			2, 4, 5,    // Triangle 4: right-top, right-bottom, bottom-right

			// Bottom section
			5, 6, 7,    // Triangle 5: bottom-right, bottom-bottom, bottom-left
			5, 7, 8,    // Triangle 6: bottom-right, bottom-left, left-bottom

			// Left section
			8, 9, 10,   // Triangle 7: left-bottom, left-left, left-top
			8, 10, 11,  // Triangle 8: left-bottom, left-top, top-left

			// Center section
			2, 5, 8,    // Triangle 9: right-top, bottom-right, left-bottom
			2, 8, 11    // Triangle 10: right-top, left-bottom, left-top
		};

		indexCount = sizeof(indices) / sizeof(indices[0]);

		// Create and fill the index buffer
		GvkHelper::create_buffer(physicalDevice, device, sizeof(indices),
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &indexHandle, &indexData);
		GvkHelper::write_to_buffer(device, indexData, indices, sizeof(indices));
	}
	

	void CompileShaders()
	{
		// Intialize runtime shader compiler HLSL -> SPIRV
		shaderc_compiler_t compiler = shaderc_compiler_initialize();
		shaderc_compile_options_t options = CreateCompileOptions();

		CompileVertexShader(compiler, options);
		CompileFragmentShader(compiler, options);
		CompileOutlineFragmentShader(compiler, options);
		
		// Free runtime shader compiler resources
		shaderc_compile_options_release(options);
		shaderc_compiler_release(compiler);
	}

	shaderc_compile_options_t CreateCompileOptions()
	{
		shaderc_compile_options_t retval = shaderc_compile_options_initialize();
		shaderc_compile_options_set_source_language(retval, shaderc_source_language_hlsl);
		shaderc_compile_options_set_invert_y(retval, true);
#ifndef NDEBUG
		shaderc_compile_options_set_generate_debug_info(retval);
#endif
	return retval;
	}

	void BindShutdownCallback()
	{
		// GVulkanSurface will inform us when to release any allocated resources
		shutdown.Create(vlk, [&]() {
			if (+shutdown.Find(GW::GRAPHICS::GVulkanSurface::Events::RELEASE_RESOURCES, true)) {
				CleanUp(); // unlike D3D we must be careful about destroy timing
			}
			});
	}

	void CompileOutlineFragmentShader(const shaderc_compiler_t& compiler, const shaderc_compile_options_t& options)
	{
		std::string fragmentShaderSource = ReadFileIntoString("../OutlineFragmentShader.hlsl");

		shaderc_compilation_result_t result = shaderc_compile_into_spv(
			compiler, fragmentShaderSource.c_str(), fragmentShaderSource.length(),
			shaderc_fragment_shader, "outline.frag", "main", options);

		if (shaderc_result_get_compilation_status(result) != shaderc_compilation_status_success)
		{
			PrintLabeledDebugString("Outline Fragment Shader Errors:\n", shaderc_result_get_error_message(result));
			abort();
			return;
		}

		GvkHelper::create_shader_module(device, shaderc_result_get_length(result),
			(char*)shaderc_result_get_bytes(result), &outlineFragmentShader);
		shaderc_result_release(result);
	}

	void CompileVertexShader(const shaderc_compiler_t& compiler, const shaderc_compile_options_t& options)
	{
		std::string vertexShaderSource = ReadFileIntoString("../VertexShader.hlsl");

		shaderc_compilation_result_t result = shaderc_compile_into_spv( // compile
			compiler, vertexShaderSource.c_str(), vertexShaderSource.size(),
			shaderc_vertex_shader, "main.vert", "main", options);

		if (shaderc_result_get_compilation_status(result) != shaderc_compilation_status_success) // errors?
		{
			PrintLabeledDebugString("Vertex Shader Errors:\n", shaderc_result_get_error_message(result));
			abort();
			return;
		}

		GvkHelper::create_shader_module(device, shaderc_result_get_length(result), // load into Vulkan
			(char*)shaderc_result_get_bytes(result), &vertexShader);
		shaderc_result_release(result); // done
	}

	void CompileFragmentShader(const shaderc_compiler_t& compiler, const shaderc_compile_options_t& options)
	{
		std::string fragmentShaderSource = ReadFileIntoString("../FragmentShader.hlsl");

		shaderc_compilation_result_t result = shaderc_compile_into_spv( // compile
			compiler, fragmentShaderSource.c_str(), fragmentShaderSource.length(),
			shaderc_fragment_shader, "main.frag", "main", options);

		if (shaderc_result_get_compilation_status(result) != shaderc_compilation_status_success) // errors?
		{
			PrintLabeledDebugString("Fragment Shader Errors:\n", shaderc_result_get_error_message(result));
			abort();
			return;
		}

		GvkHelper::create_shader_module(device, shaderc_result_get_length(result), // load into Vulkan
			(char*)shaderc_result_get_bytes(result), &fragmentShader);
		shaderc_result_release(result); // done
	}

	void InitializeGraphicsPipeline()
	{
		// Setup shader stages for standard pipeline (colored triangles)
		VkPipelineShaderStageCreateInfo triangle_stage_create_info[2] = {};

		// Create Stage Info for Vertex Shader
		triangle_stage_create_info[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		triangle_stage_create_info[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		triangle_stage_create_info[0].module = vertexShader; // TODO: Part 4f
		triangle_stage_create_info[0].pName = "main";

		// Create Stage Info for Fragment Shader
		triangle_stage_create_info[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		triangle_stage_create_info[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		triangle_stage_create_info[1].module = fragmentShader; // TODO: Part 4f, Part 4g
		triangle_stage_create_info[1].pName = "main";

		// Setup shader stages for outline pipeline (white lines)
		VkPipelineShaderStageCreateInfo outline_stage_create_info[2] = {};

		// Vertex shader stage (same as triangle pipeline)
		outline_stage_create_info[0] = triangle_stage_create_info[0];

		// Fragment shader stage for outlines (white)
		outline_stage_create_info[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		outline_stage_create_info[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		outline_stage_create_info[1].module = outlineFragmentShader;
		outline_stage_create_info[1].pName = "main";
	
		VkPipelineInputAssemblyStateCreateInfo assembly_create_info = CreateVkPipelineInputAssemblyStateCreateInfo();

		VkVertexInputBindingDescription vertex_binding_description = CreateVkVertexInputBindingDescription();
		VkVertexInputAttributeDescription vertex_attribute_descriptions[2]; //TODO: Part 4f

		vertex_attribute_descriptions[0].binding = 0;
		vertex_attribute_descriptions[0].location = 0;
		vertex_attribute_descriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		vertex_attribute_descriptions[0].offset = 0;

		vertex_attribute_descriptions[1].binding = 0;
		vertex_attribute_descriptions[1].location = 1;
		vertex_attribute_descriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		vertex_attribute_descriptions[1].offset = offsetof(Vertex, r);

		VkPipelineVertexInputStateCreateInfo input_vertex_info =
			CreateVkPipelineVertexInputStateCreateInfo(
				&vertex_binding_description, 1,
				vertex_attribute_descriptions, 2);

		VkViewport viewport = CreateViewportFromWindowDimensions();
		VkRect2D scissor = CreateScissorFromWindowDimensions();
		VkPipelineViewportStateCreateInfo viewport_create_info = CreateVkPipelineViewportStateCreateInfo(&viewport, 1, &scissor, 1);
		VkPipelineRasterizationStateCreateInfo rasterization_create_info = CreateVkPipelineRasterizationStateCreateInfo();
		VkPipelineMultisampleStateCreateInfo multisample_create_info = CreateVkPipelineMultisampleStateCreateInfo();
		VkPipelineDepthStencilStateCreateInfo depth_stencil_create_info = CreateVkPipelineDepthStencilStateCreateInfo();
		VkPipelineColorBlendAttachmentState color_blend_attachment_state = CreateVkPipelineColorBlendAttachmentState();
		VkPipelineColorBlendStateCreateInfo color_blend_create_info = CreateVkPipelineColorBlendStateCreateInfo(&color_blend_attachment_state, 1);

		VkDynamicState dynamic_states[2] = 
		{
			// By setting these we do not need to re-create the pipeline on Resize
			VK_DYNAMIC_STATE_VIEWPORT, 
			VK_DYNAMIC_STATE_SCISSOR
		};

		// Create depth stencil state for triangles (normal depth test)
		VkPipelineDepthStencilStateCreateInfo triangle_depth_stencil_create_info = CreateVkPipelineDepthStencilStateCreateInfo();

		// Create rasterization state for outlines (with slight bias to prevent z-fighting)
		VkPipelineRasterizationStateCreateInfo outline_rasterization_create_info = rasterization_create_info;
		outline_rasterization_create_info.depthBiasEnable = VK_TRUE;
		outline_rasterization_create_info.depthBiasConstantFactor = -1.0f;

		VkPipelineDynamicStateCreateInfo dynamic_create_info = CreateVkPipelineDynamicStateCreateInfo(dynamic_states, 2);
		CreatePipelineLayout();

		VkPipelineInputAssemblyStateCreateInfo line_assembly_create_info = CreateVkPipelineInputAssemblyStateCreateInfo();
		line_assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;

		VkPipelineInputAssemblyStateCreateInfo triangle_assembly_create_info = line_assembly_create_info;
		triangle_assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

		// Pipeline State... (FINALLY) 
		VkGraphicsPipelineCreateInfo triangle_pipeline_create_info = {};
		triangle_pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		triangle_pipeline_create_info.stageCount = 2;
		triangle_pipeline_create_info.pStages = triangle_stage_create_info;
		triangle_pipeline_create_info.pInputAssemblyState = &assembly_create_info;
		triangle_pipeline_create_info.pVertexInputState = &input_vertex_info;
		triangle_pipeline_create_info.pViewportState = &viewport_create_info;
		triangle_pipeline_create_info.pRasterizationState = &rasterization_create_info;
		triangle_pipeline_create_info.pMultisampleState = &multisample_create_info;
		triangle_pipeline_create_info.pDepthStencilState = &depth_stencil_create_info;
		triangle_pipeline_create_info.pColorBlendState = &color_blend_create_info;
		triangle_pipeline_create_info.pDynamicState = &dynamic_create_info;
		triangle_pipeline_create_info.layout = pipelineLayout;
		triangle_pipeline_create_info.renderPass = renderPass;
		triangle_pipeline_create_info.subpass = 0;
		triangle_pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
		vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1,
			&triangle_pipeline_create_info, nullptr, &pipeline);

		// TODO: Part 3a
		triangle_assembly_create_info = assembly_create_info; 
		triangle_assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

		triangle_pipeline_create_info.pInputAssemblyState = &triangle_assembly_create_info;

		vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1,
			&triangle_pipeline_create_info, nullptr, &trianglePipeline);

		// TODO: Part 4g
		vkDestroyPipeline(device, pipeline, nullptr);

		VkGraphicsPipelineCreateInfo outline_pipeline_create_info = triangle_pipeline_create_info;
		outline_pipeline_create_info.pStages = outline_stage_create_info;
		outline_pipeline_create_info.pInputAssemblyState = &line_assembly_create_info;
		outline_pipeline_create_info.pRasterizationState = &outline_rasterization_create_info; 

		vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1,
			&outline_pipeline_create_info, nullptr, &pipeline);
	}



	VkPipelineInputAssemblyStateCreateInfo CreateVkPipelineInputAssemblyStateCreateInfo()
	{
		VkPipelineInputAssemblyStateCreateInfo retval = {};

		retval.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		retval.topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP; // TODO: Part 1b
		retval.primitiveRestartEnable = false;

		return retval;
	}

	VkVertexInputBindingDescription CreateVkVertexInputBindingDescription()
	{
		VkVertexInputBindingDescription retval = {};

		retval.binding = 0;
		retval.stride = sizeof(Vertex); // TODO: Part 4f 
		retval.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return retval;
	}

	VkPipelineVertexInputStateCreateInfo CreateVkPipelineVertexInputStateCreateInfo(
	VkVertexInputBindingDescription* bindingDescriptions, uint32_t bindingCount,
		VkVertexInputAttributeDescription* attributeDescriptions, uint32_t attributeCount)
	{
		VkPipelineVertexInputStateCreateInfo retval = {};

		retval.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		retval.vertexBindingDescriptionCount = bindingCount; // TODO: Part 4f
		retval.pVertexBindingDescriptions = bindingDescriptions;
		retval.vertexAttributeDescriptionCount = attributeCount;
		retval.pVertexAttributeDescriptions = attributeDescriptions;

		return retval;
	}

	VkViewport CreateViewportFromWindowDimensions()
	{
		VkViewport retval = {};

		retval.x = 0;
		retval.y = 0;
		retval.width = static_cast<float>(windowWidth);
		retval.height = static_cast<float>(windowHeight);
		retval.minDepth = 0;
		retval.maxDepth = 1;

		return retval;
	}

	VkRect2D CreateScissorFromWindowDimensions()
	{
		VkRect2D retval = {};

		retval.offset.x = 0;
		retval.offset.y = 0;
		retval.extent.width = windowWidth;
		retval.extent.height = windowHeight;

		return retval;
	}

	VkPipelineViewportStateCreateInfo CreateVkPipelineViewportStateCreateInfo(VkViewport* viewports, uint32_t viewportCount, VkRect2D* scissors, uint32_t scissorCount)
	{
		VkPipelineViewportStateCreateInfo retval = {};

		retval.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		retval.viewportCount = viewportCount;
		retval.pViewports = viewports;
		retval.scissorCount = scissorCount;
		retval.pScissors = scissors;

		return retval;
	}

	VkPipelineRasterizationStateCreateInfo CreateVkPipelineRasterizationStateCreateInfo()
	{
		VkPipelineRasterizationStateCreateInfo retval = {};

		retval.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		retval.rasterizerDiscardEnable = VK_FALSE;
		retval.polygonMode = VK_POLYGON_MODE_FILL;
		retval.lineWidth = 1.0f;
		retval.cullMode = VK_CULL_MODE_BACK_BIT;
		retval.frontFace = VK_FRONT_FACE_CLOCKWISE;
		retval.depthClampEnable = VK_FALSE;
		retval.depthBiasEnable = VK_FALSE;
		retval.depthBiasClamp = 0.0f;
		retval.depthBiasConstantFactor = 0.0f;
		retval.depthBiasSlopeFactor = 0.0f;

		return retval;
	}

	VkPipelineMultisampleStateCreateInfo CreateVkPipelineMultisampleStateCreateInfo()
	{
		VkPipelineMultisampleStateCreateInfo retval = {};

		retval.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		retval.sampleShadingEnable = VK_FALSE;
		retval.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		retval.minSampleShading = 1.0f;
		retval.pSampleMask = VK_NULL_HANDLE;
		retval.alphaToCoverageEnable = VK_FALSE;
		retval.alphaToOneEnable = VK_FALSE;

		return retval;
	}

	VkPipelineDepthStencilStateCreateInfo CreateVkPipelineDepthStencilStateCreateInfo()
	{
		VkPipelineDepthStencilStateCreateInfo retval = {};

		retval.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		retval.depthTestEnable = VK_TRUE; // TODO: Part 4g
		retval.depthWriteEnable = VK_TRUE;
		retval.depthCompareOp = VK_COMPARE_OP_LESS;
		retval.depthBoundsTestEnable = VK_FALSE;
		retval.minDepthBounds = 0.0f;
		retval.maxDepthBounds = 1.0f;
		retval.stencilTestEnable = VK_FALSE;

		return retval;
	}

	VkPipelineColorBlendAttachmentState CreateVkPipelineColorBlendAttachmentState()
	{
		VkPipelineColorBlendAttachmentState retval;

		retval.colorWriteMask = 0xF;
		retval.blendEnable = VK_FALSE;
		retval.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
		retval.dstColorBlendFactor = VK_BLEND_FACTOR_DST_COLOR;
		retval.colorBlendOp = VK_BLEND_OP_ADD;
		retval.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		retval.dstAlphaBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;
		retval.alphaBlendOp = VK_BLEND_OP_ADD;

		return retval;
	}

	VkPipelineColorBlendStateCreateInfo CreateVkPipelineColorBlendStateCreateInfo(VkPipelineColorBlendAttachmentState* attachmentStates, uint32_t attachmentCount)
	{
		VkPipelineColorBlendStateCreateInfo retval = {};

		retval.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		retval.logicOpEnable = VK_FALSE;
		retval.logicOp = VK_LOGIC_OP_COPY;
		retval.attachmentCount = attachmentCount;
		retval.pAttachments = attachmentStates;
		retval.blendConstants[0] = 0.0f;
		retval.blendConstants[1] = 0.0f;
		retval.blendConstants[2] = 0.0f;
		retval.blendConstants[3] = 0.0f;

		return retval;
	}

	VkPipelineDynamicStateCreateInfo CreateVkPipelineDynamicStateCreateInfo(VkDynamicState* dynamicStates, uint32_t dynamicStateCount)
	{
		VkPipelineDynamicStateCreateInfo retval = {};

		retval.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		retval.dynamicStateCount = dynamicStateCount;
		retval.pDynamicStates = dynamicStates;

		return retval;
	}

	void CreatePipelineLayout()
	{
		// TODO: Part 2c
		VkPushConstantRange pushConstantRange = {};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; 
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(float) * 32;
		// Descriptor pipeline layout
		VkPipelineLayoutCreateInfo pipeline_layout_create_info = {};
		pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipeline_layout_create_info.setLayoutCount = 0;
		pipeline_layout_create_info.pSetLayouts = VK_NULL_HANDLE;
		pipeline_layout_create_info.pushConstantRangeCount = 1; // TODO: Part 2c
		pipeline_layout_create_info.pPushConstantRanges = &pushConstantRange; // TODO: Part 2c
		vkCreatePipelineLayout(device, &pipeline_layout_create_info,
			nullptr, &pipelineLayout);
	}

	struct Vertex
	{
		float x, y;
		float r, g, b, a;
	};

	struct SHADER_VARS
	{
		float worldMatrix[16];
		float padding[16];
	};

public:
	SHADER_VARS shaderData = {};

	void Render()
	{
		// TODO: Part 2a
		auto now = std::chrono::steady_clock::now();
		float seconds = std::chrono::duration<float>(now - startTime).count();

		GW::MATH::GMATRIXF identityMatrix;
		mMatrix.IdentityF(identityMatrix);

		mMatrix.RotateZLocalF(identityMatrix, seconds, rotationMatrix); // Rotate the triangle a little bit each frame
		// TODO: Part 2b
		memcpy(shaderData.worldMatrix, rotationMatrix.data, sizeof(float) * 16);

		VkCommandBuffer commandBuffer = GetCurrentCommandBuffer();
		SetUpPipeline(commandBuffer);
		// TODO: Part 3b

		// TODO: Part 3d
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, trianglePipeline);
		BindIndexBuffer(commandBuffer);
		vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0); 

		// TODO: Part 4g
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
		vkCmdDraw(commandBuffer, 13, 1, 0, 0); 
	}

private:
	VkCommandBuffer GetCurrentCommandBuffer()
	{
		unsigned int currentBuffer;
		vlk.GetSwapchainCurrentImage(currentBuffer);
		VkCommandBuffer retval;
		vlk.GetCommandBuffer(currentBuffer, (void**)&retval);
		return retval;
	}

	void SetUpPipeline(const VkCommandBuffer& commandBuffer)
	{
		UpdateWindowDimensions();
		SetViewport(commandBuffer);
		SetScissor(commandBuffer);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline); // TODO: Part 4g
		// TODO: Part 2d
		vkCmdPushConstants(
			commandBuffer, 
			pipelineLayout,
			VK_SHADER_STAGE_VERTEX_BIT,
			0,
			sizeof(SHADER_VARS),
			&shaderData
		); 
		BindVertexBuffers(commandBuffer);
	}

	void SetScissor(const VkCommandBuffer& commandBuffer)
	{
		VkRect2D scissor = CreateScissorFromWindowDimensions();
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}

	void SetViewport(const VkCommandBuffer& commandBuffer)
	{
		VkViewport viewport = CreateViewportFromWindowDimensions();
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	}

	void BindVertexBuffers(const VkCommandBuffer& commandBuffer)
	{
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexHandle, offsets);
	}

	void BindIndexBuffer(const VkCommandBuffer& commandBuffer)
	{
		vkCmdBindIndexBuffer(commandBuffer, indexHandle, 0, VK_INDEX_TYPE_UINT16);
	}

	void CleanUp()
	{
		// wait till everything has completed
		vkDeviceWaitIdle(device);
		// Release allocated buffers, shaders & pipeline
		vkDestroyBuffer(device, vertexHandle, nullptr);
		vkFreeMemory(device, vertexData, nullptr);
		vkDestroyShaderModule(device, vertexShader, nullptr);
		vkDestroyShaderModule(device, fragmentShader, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		vkDestroyPipeline(device, pipeline, nullptr);
		// TODO: Part 3a
		vkDestroyPipeline(device, trianglePipeline, nullptr);
		// TODO: Part 3c
		vkDestroyBuffer(device, indexHandle, nullptr);
		vkFreeMemory(device, indexData, nullptr);
		// TODO: Part 4b
		vkDestroyShaderModule(device, outlineFragmentShader, nullptr);
	}
};
