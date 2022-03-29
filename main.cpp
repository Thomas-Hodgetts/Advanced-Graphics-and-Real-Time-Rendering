#include "stdafx.h"

//https://stackoverflow.com/questions/13445688/how-to-generate-a-random-number-in-c
int GenerateRandomNumber(int LowerBounds, int UpperBounds)
{
	std::random_device rd;
	std::mt19937::result_type seed = rd() ^ ((std::mt19937::result_type)std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count() + (std::mt19937::result_type)std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count());

	std::mt19937 gen(seed);
	std::uniform_int_distribution<unsigned> distrib(LowerBounds, UpperBounds);

	return distrib(gen);
}

int WINAPI WinMain(HINSTANCE hInstance,    //Main windows function
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nShowCmd){
	// create the window
	if (!InitializeWindow(hInstance, nShowCmd, FullScreen))
	{
		MessageBox(0, L"Window Initialization - Failed",
			L"Error", MB_OK);
		return 1;
	}

	// initialize direct3d
	if (!InitD3D())
	{
		MessageBox(0, L"Failed to initialize direct3d 12",
			L"Error", MB_OK);
		Cleanup();
		return 1;
	}

	// start the main loop
	mainloop();

	// we want to wait for the gpu to finish executing the command list before we start releasing everything
	WaitForPreviousFrame();

	// close the fence event
	CloseHandle(fenceEvent);

	m_GameManager.Destory();

	// clean up everything
	Cleanup();

	return 0;
}

// create and show the window
bool InitializeWindow(HINSTANCE hInstance,
	int ShowWnd,
	bool fullscreen)

{
	if (fullscreen)
	{
		HMONITOR hmon = MonitorFromWindow(hwnd,
			MONITOR_DEFAULTTONEAREST);
		MONITORINFO mi = { sizeof(mi) };
		GetMonitorInfo(hmon, &mi);

		Width = mi.rcMonitor.right - mi.rcMonitor.left;
		Height = mi.rcMonitor.bottom - mi.rcMonitor.top;
	}

	WNDCLASSEX wc;

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = NULL;
	wc.cbWndExtra = NULL;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = WindowName;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);



	if (!RegisterClassEx(&wc))
	{
		MessageBox(NULL, L"Error registering class",
			L"Error", MB_OK | MB_ICONERROR);
		return false;
	}

	hwnd = CreateWindowEx(NULL,
		WindowName,
		WindowTitle,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		Width, Height,
		NULL,
		NULL,
		hInstance,
		NULL);

	if (!hwnd)
	{
		MessageBox(NULL, L"Error creating window",
			L"Error", MB_OK | MB_ICONERROR);
		return false;
	}

	if (fullscreen)
	{
		SetWindowLong(hwnd, GWL_STYLE, 0);
	}

	ShowWindow(hwnd, ShowWnd);
	UpdateWindow(hwnd);

	return true;
}

void mainloop() {
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	while (Running)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			// run game code
			Update(); // update the game logic
			Render(); // execute the command queue (rendering the scene is the result of the gpu executing the command lists)
		}
	}
}

LRESULT CALLBACK WndProc(HWND hwnd,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam)

{
	switch (msg)
	{
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE) {
			if (MessageBox(0, L"Are you sure you want to exit?",
				L"Really?", MB_YESNO | MB_ICONQUESTION) == IDYES)
			{
				Running = false;
				DestroyWindow(hwnd);
			}
		}
		return 0;

	case WM_DESTROY: // x button on top right corner of window was pressed
		Running = false;
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd,
		msg,
		wParam,
		lParam);
}

BYTE* imageData;

bool InitD3D()
{
	std::fstream newfile;
	newfile.open("settings.txt", ios::in); //open a file to perform read operation using file object
	if (newfile.is_open())
	{
		std::string text;
		std::getline(newfile, text);
		if (text == "ShadowMapping")
		{
			m_RenderToTexture = false;
			m_TextureSetUp = false;
			m_ShadowMapping = true;
			m_Rotate = false;
		}
		else if (text == "RenderToTexture")
		{
			m_RenderToTexture = true;
			m_TextureSetUp = true;
			m_ShadowMapping = false;
			m_Rotate = false;
		}
		else if (text == "ParallaxMapping")
		{
			m_RenderToTexture = false;
			m_TextureSetUp = false;
			m_ShadowMapping = false;
			m_Rotate = false;
		}
		else if (text == "NormalMapping")
		{
			m_RenderToTexture = false;
			m_TextureSetUp = false;
			m_ShadowMapping = false;
			m_Rotate = false;
			m_NormalMapping = true;
		}
		else
		{
			m_RenderToTexture = false;
			m_TextureSetUp = false;
			m_ShadowMapping = false;
			m_Rotate = false;
		}
	}
	else
	{
		m_RenderToTexture = false;
		m_TextureSetUp = false;
		m_ShadowMapping = false;
		m_Rotate = false;
	}

	HRESULT hr;

	// -- Create the Swap Chain (double/tripple buffering) -- //

	DXGI_MODE_DESC backBufferDesc = {}; // this is to describe our display mode
	backBufferDesc.Width = Width; // buffer width
	backBufferDesc.Height = Height; // buffer height
	backBufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // format of the buffer (rgba 32 bits, 8 bits for each chanel)

														// describe our multi-sampling. We are not multi-sampling, so we set the count to 1 (we need at least one sample of course)
	DXGI_SAMPLE_DESC sampleDesc = {};
	sampleDesc.Count = 1; // multisample count (no multisampling, so we just put 1, since we still need 1 sample)
	sampleDesc.Quality = 0;

						  // Describe and create the swap chain.
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferCount = frameBufferCount; // number of buffers we have
	swapChainDesc.BufferDesc = backBufferDesc; // our back buffer description
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // this says the pipeline will render to this swap chain
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // dxgi will discard the buffer (data) after we call present
	swapChainDesc.OutputWindow = hwnd; // handle to our window
	swapChainDesc.SampleDesc = sampleDesc; // our multi-sampling description
	swapChainDesc.Windowed = !FullScreen; // set to true, then if in fullscreen must call SetFullScreenState with true for full screen to get uncapped fps

	IDXGISwapChain* tempSwapChain;

	// -- Create the Back Buffers (render target views) Descriptor Heap -- //

	// describe an rtv descriptor heap and create
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = frameBufferCount;// + 3; // number of descriptors for this heap.
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; // this heap is a render target view heap

													   // This heap will not be directly referenced by the shaders (not shader visible), as this will store the output from the pipeline
													   // otherwise we would set the heap's flag to D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	m_GameManager = GraphicsManager(1920, 1080);
	OutputManager output(&m_GameManager, swapChainDesc, rtvHeapDesc);

	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;


	// create a static sampler
	D3D12_STATIC_SAMPLER_DESC sampler = {};
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler.MipLODBias = 0;
	sampler.MaxAnisotropy = 0;
	sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	sampler.MinLOD = 0.0f;
	sampler.MaxLOD = D3D12_FLOAT32_MAX;
	sampler.ShaderRegister = 0;
	sampler.RegisterSpace = 0;
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	const CD3DX12_STATIC_SAMPLER_DESC shadow(
		1,
		D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT,
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,
		0.0f,
		16,
		D3D12_COMPARISON_FUNC_LESS_EQUAL,
		D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK);

	D3D12_STATIC_SAMPLER_DESC samples[2]{ sampler , shadow };


	m_GameManager.CreateRootSignature(rootSignatureFlags, samples, 2, 5, 1, 4, L"Pipeline1");
	m_GameManager.CompileVertexShader(L"Pipeline1", L"VertexShader.hlsl", "main");
	m_GameManager.CompilePixelShader(L"Pipeline1", L"PixelShader.hlsl", "main");
	m_GameManager.CompileDomainShader(L"Pipeline1", L"DomainShader.hlsl", "main");
	m_GameManager.CompileHullShader(L"Pipeline1", L"HullShader.hlsl", "main");

	m_GameManager.CreateRootSignature(rootSignatureFlags, samples, 2, 10, 1, 9, L"Pipeline2");
	m_GameManager.CompileVertexShader(L"Pipeline2", L"VertexShader.hlsl", "main");
	m_GameManager.CompilePixelShader(L"Pipeline2", L"TerrainPixelShader.hlsl", "main");
	m_GameManager.CompileDomainShader(L"Pipeline2", L"DomainShader.hlsl", "main");
	m_GameManager.CompileHullShader(L"Pipeline2", L"HullShader.hlsl", "main");



	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	D3D12_INPUT_ELEMENT_DESC inputLayout2[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};


	m_GameManager.AddFrameInputLayout(inputLayout, L"Pipeline1", 5);
	m_GameManager.AddFrameInputLayout(inputLayout, L"Pipeline2", 5);
	m_GameManager.AddFrameInputLayout(inputLayout2, L"OutputManagerInput2", 2);

	m_GameManager.CreatePipeline(L"Pipeline1");
	m_GameManager.CreatePipeline(L"Pipeline2");


	std::vector<Vertex> vertexList(36);
	vertexList[0] = { -0.5f,  0.5f, -0.5f, 0.0f, 0.0f }; //0;
	vertexList[1] = { 0.5f, -0.5f, -0.5f, 1.0f, 1.0f }; //1;
	vertexList[2] = { -0.5f, -0.5f, -0.5f, 0.0f, 1.0f }; //2;
	vertexList[3] = { -0.5f,  0.5f, -0.5f, 0.0f, 0.0f }; //0;
	vertexList[4] = { 0.5f,  0.5f, -0.5f, 1.0f, 0.0f }; //3;
	vertexList[5] = { 0.5f, -0.5f, -0.5f, 1.0f, 1.0f }; //1;
	vertexList[6] = { 0.5f, -0.5f, -0.5f, 0.0f, 1.0f }; //4;
	vertexList[7] = { 0.5f,  0.5f,  0.5f, 1.0f, 0.0f }; //5;
	vertexList[8] = { 0.5f, -0.5f,  0.5f, 1.0f, 1.0f }; //6;
	vertexList[9] = { 0.5f, -0.5f, -0.5f, 0.0f, 1.0f }; //4;
	vertexList[10] = { 0.5f,  0.5f, -0.5f, 0.0f, 0.0f }; //7;
	vertexList[11] = { 0.5f,  0.5f,  0.5f, 1.0f, 0.0f }; //5;
	vertexList[12] = { -0.5f,  0.5f,  0.5f, 0.0f, 0.0f }; //8;
	vertexList[13] = { -0.5f, -0.5f, -0.5f, 1.0f, 1.0f }; //9;
	vertexList[14] = { -0.5f, -0.5f,  0.5f, 0.0f, 1.0f }; //10;
	vertexList[15] = { -0.5f,  0.5f,  0.5f, 0.0f, 0.0f }; //8;
	vertexList[16] = { -0.5f,  0.5f, -0.5f, 1.0f, 0.0f }; //11;
	vertexList[17] = { -0.5f, -0.5f, -0.5f, 1.0f, 1.0f }; //9;
	vertexList[18] = { 0.5f,  0.5f,  0.5f, 0.0f, 0.0f }; //12;
	vertexList[19] = { -0.5f, -0.5f,  0.5f, 1.0f, 1.0f }; //13;
	vertexList[20] = { 0.5f, -0.5f,  0.5f, 0.0f, 1.0f }; //14;
	vertexList[21] = { 0.5f,  0.5f,  0.5f, 0.0f, 0.0f }; //12;
	vertexList[22] = { -0.5f,  0.5f,  0.5f, 1.0f, 0.0f }; //15;
	vertexList[23] = { -0.5f, -0.5f,  0.5f, 1.0f, 1.0f }; //13;
	vertexList[24] = { -0.5f,  0.5f, -0.5f, 0.0f, 1.0f }; //16;
	vertexList[25] = { 0.5f,  0.5f,  0.5f, 1.0f, 0.0f }; //17;
	vertexList[26] = { 0.5f,  0.5f, -0.5f, 1.0f, 1.0f }; //18;
	vertexList[27] = { -0.5f,  0.5f, -0.5f, 0.0f, 1.0f }; //16;
	vertexList[28] = { -0.5f,  0.5f,  0.5f, 0.0f, 0.0f }; //19;
	vertexList[29] = { 0.5f,  0.5f,  0.5f, 1.0f, 0.0f }; //17;
	vertexList[30] = { 0.5f, -0.5f,  0.5f, 0.0f, 0.0f }; //20;
	vertexList[31] = { -0.5f, -0.5f, -0.5f, 1.0f, 1.0f }; //21;
	vertexList[32] = { 0.5f, -0.5f, -0.5f, 0.0f, 1.0f }; //22;
	vertexList[33] = { 0.5f, -0.5f,  0.5f, 0.0f, 0.0f }; //20;
	vertexList[34] = { -0.5f, -0.5f,  0.5f, 1.0f, 0.0f }; //23;
	vertexList[35] = { -0.5f, -0.5f, -0.5f, 1.0f, 1.0f }; //21

	std::vector<DWORD> indexList(36);
	indexList[0] = 0;
	indexList[1] = 1;
	indexList[2] = 2;
	indexList[3] = 3;
	indexList[4] = 4;
	indexList[5] = 5;
	indexList[6] = 6;
	indexList[7] = 7;
	indexList[8] = 8;
	indexList[9] = 9;
	indexList[10] = 10;
	indexList[11] = 11;
	indexList[12] = 12;
	indexList[13] = 13;
	indexList[14] = 14;
	indexList[15] = 15;
	indexList[16] = 16;
	indexList[17] = 17;
	indexList[18] = 18;
	indexList[19] = 19;
	indexList[20] = 20;
	indexList[21] = 21;
	indexList[22] = 22;
	indexList[23] = 23;
	indexList[24] = 24;
	indexList[25] = 25;
	indexList[26] = 26;
	indexList[27] = 27;
	indexList[28] = 28;
	indexList[29] = 29;
	indexList[30] = 30;
	indexList[31] = 31;
	indexList[32] = 32;
	indexList[33] = 33;
	indexList[34] = 34;
	indexList[35] = 35;

	m_GameManager.CreateGeomerty(vertexList.data(), vertexList.size(), indexList.data(), indexList.size(), L"TestGeometry");
	m_Manager = new SystemManager();

	// Create vertex buffer

	Vertex vList2[] = {
		{ -1.0f, 1.0f, 0.0f , 0.0f, 0.0f }, //0
		{ 1.0f, 1.0f, 0.0f, 1.0f, 0.0f }, //1
		{ -1.0f, -1.0f, 0.0f, 0.0f, 1.0f }, //2
		{ 1.0f, -1.0f, 0.0f , 1.0f, 1.0 }, //3
	};

	// a quad

	Vertex vList[] = {
		// front face
		{ -0.5f,  0.5f, -0.5f, 0.0f, 0.0f }, //0
		{  0.5f, -0.5f, -0.5f, 1.0f, 1.0f }, //1
		{ -0.5f, -0.5f, -0.5f, 0.0f, 1.0f }, //2
		{ -0.5f,  0.5f, -0.5f, 0.0f, 0.0f }, //0
		{  0.5f,  0.5f, -0.5f, 1.0f, 0.0f }, //3
		{  0.5f, -0.5f, -0.5f, 1.0f, 1.0f }, //1
		{  0.5f, -0.5f, -0.5f, 0.0f, 1.0f }, //4
		{  0.5f,  0.5f,  0.5f, 1.0f, 0.0f }, //5
		{  0.5f, -0.5f,  0.5f, 1.0f, 1.0f }, //6
		{  0.5f, -0.5f, -0.5f, 0.0f, 1.0f }, //4
		{  0.5f,  0.5f, -0.5f, 0.0f, 0.0f }, //7
		{  0.5f,  0.5f,  0.5f, 1.0f, 0.0f }, //5
		{ -0.5f,  0.5f,  0.5f, 0.0f, 0.0f }, //8
		{ -0.5f, -0.5f, -0.5f, 1.0f, 1.0f }, //9
		{ -0.5f, -0.5f,  0.5f, 0.0f, 1.0f }, //10
		{ -0.5f,  0.5f,  0.5f, 0.0f, 0.0f }, //8
		{ -0.5f,  0.5f, -0.5f, 1.0f, 0.0f }, //11
		{ -0.5f, -0.5f, -0.5f, 1.0f, 1.0f }, //9
		{  0.5f,  0.5f,  0.5f, 0.0f, 0.0f }, //12
		{ -0.5f, -0.5f,  0.5f, 1.0f, 1.0f }, //13
		{  0.5f, -0.5f,  0.5f, 0.0f, 1.0f }, //14
		{  0.5f,  0.5f,  0.5f, 0.0f, 0.0f }, //12
		{ -0.5f,  0.5f,  0.5f, 1.0f, 0.0f }, //15
		{ -0.5f, -0.5f,  0.5f, 1.0f, 1.0f }, //13
		{ -0.5f,  0.5f, -0.5f, 0.0f, 1.0f }, //16
		{  0.5f,  0.5f,  0.5f, 1.0f, 0.0f }, //17
		{  0.5f,  0.5f, -0.5f, 1.0f, 1.0f }, //18
		{ -0.5f,  0.5f, -0.5f, 0.0f, 1.0f }, //16
		{ -0.5f,  0.5f,  0.5f, 0.0f, 0.0f }, //19
		{  0.5f,  0.5f,  0.5f, 1.0f, 0.0f }, //17
		{  0.5f, -0.5f,  0.5f, 0.0f, 0.0f }, //20
		{ -0.5f, -0.5f, -0.5f, 1.0f, 1.0f }, //21
		{  0.5f, -0.5f, -0.5f, 0.0f, 1.0f }, //22
		{  0.5f, -0.5f,  0.5f, 0.0f, 0.0f }, //20
		{ -0.5f, -0.5f,  0.5f, 1.0f, 0.0f }, //23
		{ -0.5f, -0.5f, -0.5f, 1.0f, 1.0f }, //21
	};

	// a quad (2 triangles)
	DWORD iList[] = {
		// ffront face
		0, 1, 2, // first triangle
		3, 4, 5, // second triangle

		// left face
		6, 7, 8, // first triangle
		9, 10, 11, // second triangle

		// right face
		12, 13, 14, // first triangle
		15, 16, 17, // second triangle

		// back face
		18, 19, 20, // first triangle
		21, 22, 23, // second triangle

		// top face
		24, 25, 26, // first triangle
		27, 28, 29, // second triangle

		// bottom face
		30, 31, 32, // first triangle
		33, 34, 35, // second triangle
	};

	DWORD iList2[] = {
		// ffront face
		0, 1, 2, // first triangle
		0, 3, 2, // second triangle
		0, 1, 3, // thrid triangle
	};


	int iBufferSize = sizeof(iList);
	int iBufferSize2 = sizeof(iList2);

	numCubeIndices = sizeof(iList) / sizeof(DWORD);
	numCubeIndices2 = sizeof(iList2) / sizeof(DWORD);
	NormalCalculations::CalculateObjectNormals(vList, iList, numCubeIndices / 3);
	NormalCalculations::CalculateObjectNormals(vList2, iList2, numCubeIndices2 / 3);

	int i = sizeof(vList) / sizeof(Vertex);
	int i2 = sizeof(vList2) / sizeof(Vertex);
	NormalCalculations::CalculateModelVectors(vList, i);
	NormalCalculations::CalculateModelVectors(vList2, i2);


	int vBufferSize = sizeof(vList);
	int vBufferSize2 = sizeof(vList2);

	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
	depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

	D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
	depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
	depthOptimizedClearValue.DepthStencil.Stencil = 0;

	bool test = m_GameManager.CreateStencilDepthView(L"OutputManager Depth Stencil", 1,&depthStencilDesc, &depthOptimizedClearValue);


	m_GameManager.CreateConstantBuffer(L"TestGeometry", output.GetBufferSize() , 1024, sizeof(ConstantBufferPerObject), 4);


	LPCWSTR filename[4] = { L"color.jpg" ,L"normals.jpg" ,L"displacement.jpg" ,L"dx12.jpg" };
	int objectCount = sizeof(filename) / sizeof(LPCWSTR);

	m_GameManager.CreateTextureHeap(filename, objectCount, L"TestGeometry");


	// Fill out the Viewport
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (float)Width;
	viewport.Height = (float)Height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	// Fill out a scissor rect
	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right = Width;
	scissorRect.bottom = Height;

	// build projection and view matrix
	XMMATRIX tmpMat = XMMatrixPerspectiveFovLH(45.0f*(3.14f/180.0f), (float)Width / (float)Height, 0.1f, 1000.0f);
	XMStoreFloat4x4(&cameraProjMat, tmpMat);

	// set starting camera state
	cameraPosition = XMFLOAT4(0.0f, 9.0f, -6.0f, 0.0f);
	cameraTarget = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	cameraUp = XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f);

	m_Manager = new SystemManager();
	m_Manager->SetUpCamera(cameraPosition, cameraTarget, cameraUp, Width, Height, 0.1, 1000);
	m_Manager->ReturnCamera()->Update();

	Transform* transform = new Transform(Vector3D(0.0f, 9.0f, 0.0f), Vector3D(0.0f, 0.0f, 0.0f), Vector3D(2.0f, 2.0f, 2.0f));
	XMStoreFloat4x4(&transform->RotationalMatrix, XMMatrixIdentity());
	CreateObjectStruct COS;
	COS.graphicsManager = &m_GameManager;
	COS.transform = transform;
	COS.objName = "Obj1";
	COS.index = m_GameManager.GetIndexBufferView(L"TestGeometry");
	COS.vertex = m_GameManager.GetVertexBufferView(L"TestGeometry");
	m_Manager->BuildObject(COS);

	shinyMaterial.AmbientMtrl = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	shinyMaterial.DiffuseMtrl = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	shinyMaterial.SpecularMtrl = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);

	// Setup the scene's light
	basicLight.AmbientLight = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	basicLight.DiffuseLight = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	basicLight.SpecularLight = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	basicLight.SpecularPower = 32.0f;
	basicLight.LightVecW = XMFLOAT3(0.0f, 7.0f, 0);

	XMMATRIX scale = XMMatrixScaling(1,1,1);
	XMMATRIX rotation = XMMatrixRotationX(0) * XMMatrixRotationY(0) * XMMatrixRotationZ(0);
	XMMATRIX translation = XMMatrixTranslation(basicLight.LightVecW.x, basicLight.LightVecW.y, basicLight.LightVecW.z);

	XMStoreFloat4x4(&m_LightMatrix, scale * rotation * translation);

	//m_Manager->CreateTerrain(200, 200, "terrain2.raw", 2, L"Terrain");
	m_Manager->CreateTerrain(2, L"Terrain");

	Terrain* pTerrain = m_Manager->GetTerrain();

	pTerrain->CreateAnchor(Vector3D(0.0f, -17.0f, 0.0f), Vector3D(0.0f, 0.0f, 0.0f), Vector3D(1.0f, 0.5f, 1.0f));

	m_GameManager.CreateGeomerty(pTerrain->GetVertexStorage(), pTerrain->GetIndexStorage(), pTerrain->GetIdentifier(), pTerrain->GetGeometry());

	m_GameManager.CreateConstantBuffer(pTerrain->GetIdentifier(), output.GetBufferSize(), 1024, sizeof(ConstantBufferPerObject), 1);

	//LPCWSTR filename2[9] = { L"Forest01.jpg" ,L"Forest02.jpg" ,L"Forest03.jpg" ,L"Rock01.jpg", L"Rock02.jpg" ,L"Rock03.jpg" ,L"Snow01.jpg" ,L"Snow02.jpg" ,L"Snow03.jpg" };
	LPCWSTR filename2[9] = { L"Forest01Small.jpg" ,L"Forest02Small.jpg" ,L"Forest03Small.jpg" ,L"Rocks01Small.jpg", L"Rocks02Small.jpg" ,L"Rocks03Small.jpg" ,L"Snow01Small.jpg" ,L"Snow02Small.jpg" ,L"Snow03Small.jpg" };

	m_GameManager.CreateTextureHeap(filename2, 9, pTerrain->GetIdentifier());

   m_OutputManager = output;

	m_GameManager.ForceCloseCommandList(L"OutputManager");

	m_GameManager.Render(m_OutputManager.GetCurrentFrameIndex(), L"OutputManager");
	
	return true;
}

void Update()
{

	GameObject* GO = dynamic_cast<GameObject*>(m_Manager->GetStoredObject(0));

	if (GetAsyncKeyState('W') & 0x8000)
		m_Manager->ReturnCamera()->Walk(0.005f);

	if (GetAsyncKeyState('S') & 0x8000)
		m_Manager->ReturnCamera()->Walk(-0.005f);

	if (GetAsyncKeyState('A') & 0x8000)
		m_Manager->ReturnCamera()->Strafe(-0.005f);

	if (GetAsyncKeyState('D') & 0x8000)
		m_Manager->ReturnCamera()->Strafe(0.005f);

	if (GetAsyncKeyState(0x26) & 0x8000)
		m_Manager->ReturnCamera()->Pitch(0.005f);

	if (GetAsyncKeyState(0x28) & 0x8000)
		m_Manager->ReturnCamera()->Pitch(-0.005f);

	if (GetAsyncKeyState(0x25) & 0x8000)
		basicLight.LightVecW.x = basicLight.LightVecW.x + -0.0005f;

	if (GetAsyncKeyState(0x27) & 0x8000)
		basicLight.LightVecW.x = basicLight.LightVecW.x + 0.0005f;

	if (GetAsyncKeyState('G') & 0x8000)
		m_Rotate = -m_Rotate;



	GO->Update(0.f);

	m_Manager->ReturnCamera()->Update();
	CameraBufferData CBD = m_Manager->ReturnCamera()->ReturnViewPlusProjection();
	XMMATRIX viewMat = XMLoadFloat4x4(&CBD.m_view); // load view matrix
	XMMATRIX projMat = XMLoadFloat4x4(&CBD.m_projection); // load projection matrix#

	/// <summary>
	/// CONSTANT BUFFER STUFF
	/// </summary>

	if (m_ShadowMapping || m_RenderToTexture)
	{
		cbPerObject.mode = 2;
	}
	else if (m_NormalMapping)
	{
		cbPerObject.mode = 1;
	}
	else
	{
		cbPerObject.mode = 0;
	}

	cbPerObject.mode = 0;

	cbPerObject.point = basicLight;
	cbPerObject.Mat = shinyMaterial;
	cbPerObject.projection = m_Manager->ReturnCamera()->ReturnViewPlusProjection().m_projection;

	XMMATRIX wvpMat = GO->m_Particle->ReturnWorldMatrix() * viewMat * projMat; // create wvp matrix
	XMMATRIX transposed = XMMatrixTranspose(wvpMat); // must transpose wvp matrix for the gpu
	XMStoreFloat4x4(&cbPerObject.wvpMat, transposed); // store transposed wvp matrix in constant buffer
	XMStoreFloat4x4(&cbPerObject.worldPos,GO->m_Particle->ReturnWorldMatrix());
	XMStoreFloat3(&cbPerObject.EyePosW, m_Manager->ReturnCamera()->GetEye());

	int currentFrameIndex = m_OutputManager.GetCurrentFrameIndex();

	cbPerObject.TessDistanceNear = 0.0f;
	cbPerObject.TessDistanceFar = 15.0f;

	m_GameManager.UpdateObjectConstantBuffer(cbPerObject, L"TestGeometry", currentFrameIndex,0);

	wvpMat = XMLoadFloat4x4(&m_Manager->GetTerrain()->ReturnAnchor()) * viewMat * projMat; // create wvp matrix
	transposed = XMMatrixTranspose(wvpMat); // must transpose wvp matrix for the gpu
	XMStoreFloat4x4(&cbPerObject.wvpMat, transposed); // store transposed wvp matrix in constant buffer

	XMStoreFloat4x4(&cbPerObject.worldPos, XMLoadFloat4x4(&m_Manager->GetTerrain()->ReturnAnchor()));

	XMStoreFloat3(&cbPerObject.EyePosW, m_Manager->ReturnCamera()->GetEye());

	cbPerObject.TessDistanceNear = 0.0f;
	cbPerObject.TessDistanceFar = 30.0f;

	m_GameManager.UpdateObjectConstantBuffer(cbPerObject, m_Manager->GetTerrain()->GetIdentifier(), currentFrameIndex, 0);


}

void UpdatePipeline()
{
	int frameIndex = m_OutputManager.GetCurrentFrameIndex();

	m_GameManager.WaitOnFrame(frameIndex);

	m_GameManager.ResetAllocator(frameIndex, L"OutputManager");

	//WHY DO I NOT HAVE TO RESET PIPELINE 2????????
	m_GameManager.ReopenAllocator(frameIndex, L"OutputManager", L"Pipeline1");

	m_GameManager.Draw(m_OutputManager.GetCurrentFrame(), m_OutputManager.GetHeap()->GetCPUAddress(frameIndex), frameIndex,L"Pipeline1", L"OutputManager Depth Stencil", L"TestGeometry", L"TestGeometry", m_Manager, false, true, false);
	m_GameManager.Draw(m_OutputManager.GetCurrentFrame(), m_OutputManager.GetHeap()->GetCPUAddress(frameIndex), frameIndex,L"Pipeline2", L"OutputManager Depth Stencil", m_Manager->GetTerrain()->GetIdentifier() , m_Manager->GetTerrain()->GetIdentifier(), m_Manager, true, false, true);
}
void Render()
{
	HRESULT hr;

	UpdatePipeline();

	m_GameManager.Render(m_OutputManager.GetCurrentFrameIndex(), L"OutputManager");

	m_OutputManager.Present();

	m_OutputManager.UpdateFrameIndex();
}

void Cleanup()
{
}

void WaitForPreviousFrame()
{
}

// get the dxgi format equivilent of a wic format
DXGI_FORMAT GetDXGIFormatFromWICFormat(WICPixelFormatGUID& wicFormatGUID)
{
	if (wicFormatGUID == GUID_WICPixelFormat128bppRGBAFloat) return DXGI_FORMAT_R32G32B32A32_FLOAT;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppRGBAHalf) return DXGI_FORMAT_R16G16B16A16_FLOAT;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppRGBA) return DXGI_FORMAT_R16G16B16A16_UNORM;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppRGBA) return DXGI_FORMAT_R8G8B8A8_UNORM;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppBGRA) return DXGI_FORMAT_B8G8R8A8_UNORM;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppBGR) return DXGI_FORMAT_B8G8R8X8_UNORM;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppRGBA1010102XR) return DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM;

	else if (wicFormatGUID == GUID_WICPixelFormat32bppRGBA1010102) return DXGI_FORMAT_R10G10B10A2_UNORM;
	else if (wicFormatGUID == GUID_WICPixelFormat16bppBGRA5551) return DXGI_FORMAT_B5G5R5A1_UNORM;
	else if (wicFormatGUID == GUID_WICPixelFormat16bppBGR565) return DXGI_FORMAT_B5G6R5_UNORM;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppGrayFloat) return DXGI_FORMAT_R32_FLOAT;
	else if (wicFormatGUID == GUID_WICPixelFormat16bppGrayHalf) return DXGI_FORMAT_R16_FLOAT;
	else if (wicFormatGUID == GUID_WICPixelFormat16bppGray) return DXGI_FORMAT_R16_UNORM;
	else if (wicFormatGUID == GUID_WICPixelFormat8bppGray) return DXGI_FORMAT_R8_UNORM;
	else if (wicFormatGUID == GUID_WICPixelFormat8bppAlpha) return DXGI_FORMAT_A8_UNORM;

	else return DXGI_FORMAT_UNKNOWN;
}

// get a dxgi compatible wic format from another wic format
WICPixelFormatGUID GetConvertToWICFormat(WICPixelFormatGUID& wicFormatGUID)
{
	if (wicFormatGUID == GUID_WICPixelFormatBlackWhite) return GUID_WICPixelFormat8bppGray;
	else if (wicFormatGUID == GUID_WICPixelFormat1bppIndexed) return GUID_WICPixelFormat32bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat2bppIndexed) return GUID_WICPixelFormat32bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat4bppIndexed) return GUID_WICPixelFormat32bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat8bppIndexed) return GUID_WICPixelFormat32bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat2bppGray) return GUID_WICPixelFormat8bppGray;
	else if (wicFormatGUID == GUID_WICPixelFormat4bppGray) return GUID_WICPixelFormat8bppGray;
	else if (wicFormatGUID == GUID_WICPixelFormat16bppGrayFixedPoint) return GUID_WICPixelFormat16bppGrayHalf;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppGrayFixedPoint) return GUID_WICPixelFormat32bppGrayFloat;
	else if (wicFormatGUID == GUID_WICPixelFormat16bppBGR555) return GUID_WICPixelFormat16bppBGRA5551;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppBGR101010) return GUID_WICPixelFormat32bppRGBA1010102;
	else if (wicFormatGUID == GUID_WICPixelFormat24bppBGR) return GUID_WICPixelFormat32bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat24bppRGB) return GUID_WICPixelFormat32bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppPBGRA) return GUID_WICPixelFormat32bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppPRGBA) return GUID_WICPixelFormat32bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat48bppRGB) return GUID_WICPixelFormat64bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat48bppBGR) return GUID_WICPixelFormat64bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppBGRA) return GUID_WICPixelFormat64bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppPRGBA) return GUID_WICPixelFormat64bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppPBGRA) return GUID_WICPixelFormat64bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat48bppRGBFixedPoint) return GUID_WICPixelFormat64bppRGBAHalf;
	else if (wicFormatGUID == GUID_WICPixelFormat48bppBGRFixedPoint) return GUID_WICPixelFormat64bppRGBAHalf;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppRGBAFixedPoint) return GUID_WICPixelFormat64bppRGBAHalf;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppBGRAFixedPoint) return GUID_WICPixelFormat64bppRGBAHalf;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppRGBFixedPoint) return GUID_WICPixelFormat64bppRGBAHalf;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppRGBHalf) return GUID_WICPixelFormat64bppRGBAHalf;
	else if (wicFormatGUID == GUID_WICPixelFormat48bppRGBHalf) return GUID_WICPixelFormat64bppRGBAHalf;
	else if (wicFormatGUID == GUID_WICPixelFormat128bppPRGBAFloat) return GUID_WICPixelFormat128bppRGBAFloat;
	else if (wicFormatGUID == GUID_WICPixelFormat128bppRGBFloat) return GUID_WICPixelFormat128bppRGBAFloat;
	else if (wicFormatGUID == GUID_WICPixelFormat128bppRGBAFixedPoint) return GUID_WICPixelFormat128bppRGBAFloat;
	else if (wicFormatGUID == GUID_WICPixelFormat128bppRGBFixedPoint) return GUID_WICPixelFormat128bppRGBAFloat;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppRGBE) return GUID_WICPixelFormat128bppRGBAFloat;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppCMYK) return GUID_WICPixelFormat32bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppCMYK) return GUID_WICPixelFormat64bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat40bppCMYKAlpha) return GUID_WICPixelFormat64bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat80bppCMYKAlpha) return GUID_WICPixelFormat64bppRGBA;

#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8) || defined(_WIN7_PLATFORM_UPDATE)
	else if (wicFormatGUID == GUID_WICPixelFormat32bppRGB) return GUID_WICPixelFormat32bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppRGB) return GUID_WICPixelFormat64bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppPRGBAHalf) return GUID_WICPixelFormat64bppRGBAHalf;
#endif

	else return GUID_WICPixelFormatDontCare;
}

// get the number of bits per pixel for a dxgi format
int GetDXGIFormatBitsPerPixel(DXGI_FORMAT& dxgiFormat)
{
	if (dxgiFormat == DXGI_FORMAT_R32G32B32A32_FLOAT) return 128;
	else if (dxgiFormat == DXGI_FORMAT_R16G16B16A16_FLOAT) return 64;
	else if (dxgiFormat == DXGI_FORMAT_R16G16B16A16_UNORM) return 64;
	else if (dxgiFormat == DXGI_FORMAT_R8G8B8A8_UNORM) return 32;
	else if (dxgiFormat == DXGI_FORMAT_B8G8R8A8_UNORM) return 32;
	else if (dxgiFormat == DXGI_FORMAT_B8G8R8X8_UNORM) return 32;
	else if (dxgiFormat == DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM) return 32;

	else if (dxgiFormat == DXGI_FORMAT_R10G10B10A2_UNORM) return 32;
	else if (dxgiFormat == DXGI_FORMAT_B5G5R5A1_UNORM) return 16;
	else if (dxgiFormat == DXGI_FORMAT_B5G6R5_UNORM) return 16;
	else if (dxgiFormat == DXGI_FORMAT_R32_FLOAT) return 32;
	else if (dxgiFormat == DXGI_FORMAT_R16_FLOAT) return 16;
	else if (dxgiFormat == DXGI_FORMAT_R16_UNORM) return 16;
	else if (dxgiFormat == DXGI_FORMAT_R8_UNORM) return 8;
	else if (dxgiFormat == DXGI_FORMAT_A8_UNORM) return 8;

	return 0; // error
}

// load and decode image from file
int LoadImageDataFromFile(BYTE** imageData, D3D12_RESOURCE_DESC& resourceDescription, LPCWSTR filename, int &bytesPerRow)
{
	HRESULT hr;

	// we only need one instance of the imaging factory to create decoders and frames
	static IWICImagingFactory *wicFactory;

	// reset decoder, frame, and converter, since these will be different for each image we load
	IWICBitmapDecoder *wicDecoder = NULL;
	IWICBitmapFrameDecode *wicFrame = NULL;
	IWICFormatConverter *wicConverter = NULL;

	bool imageConverted = false;

	if (wicFactory == NULL)
	{
		// Initialize the COM library
		CoInitialize(NULL);

		// create the WIC factory
		hr = CoCreateInstance(
			CLSID_WICImagingFactory,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_PPV_ARGS(&wicFactory)
			);
		if (FAILED(hr)) return 0;

		hr = wicFactory->CreateFormatConverter(&wicConverter);
		if (FAILED(hr)) return 0;
	}

	if (wicConverter == NULL)
	{
		hr = wicFactory->CreateFormatConverter(&wicConverter);
		if (FAILED(hr)) return 0;
	}

	// load a decoder for the image
	hr = wicFactory->CreateDecoderFromFilename(
		filename,                        // Image we want to load in
		NULL,                            // This is a vendor ID, we do not prefer a specific one so set to null
		GENERIC_READ,                    // We want to read from this file
		WICDecodeMetadataCacheOnLoad,    // We will cache the metadata right away, rather than when needed, which might be unknown
		&wicDecoder                      // the wic decoder to be created
		);
	if (FAILED(hr)) return 0;

	// get image from decoder (this will decode the "frame")
	hr = wicDecoder->GetFrame(0, &wicFrame);
	if (FAILED(hr)) return 0;

	// get wic pixel format of image
	WICPixelFormatGUID pixelFormat;
	hr = wicFrame->GetPixelFormat(&pixelFormat);
	if (FAILED(hr)) return 0;

	// get size of image
	UINT textureWidth, textureHeight;
	hr = wicFrame->GetSize(&textureWidth, &textureHeight);
	if (FAILED(hr)) return 0;

	// we are not handling sRGB types in this tutorial, so if you need that support, you'll have to figure
	// out how to implement the support yourself

	// convert wic pixel format to dxgi pixel format
	DXGI_FORMAT dxgiFormat = GetDXGIFormatFromWICFormat(pixelFormat);

	// if the format of the image is not a supported dxgi format, try to convert it
	if (dxgiFormat == DXGI_FORMAT_UNKNOWN)
	{
		// get a dxgi compatible wic format from the current image format
		WICPixelFormatGUID convertToPixelFormat = GetConvertToWICFormat(pixelFormat);

		// return if no dxgi compatible format was found
		if (convertToPixelFormat == GUID_WICPixelFormatDontCare) return 0;

		// set the dxgi format
		dxgiFormat = GetDXGIFormatFromWICFormat(convertToPixelFormat);

		// make sure we can convert to the dxgi compatible format
		BOOL canConvert = FALSE;
		hr = wicConverter->CanConvert(pixelFormat, convertToPixelFormat, &canConvert);
		if (FAILED(hr) || !canConvert) return 0;

		// do the conversion (wicConverter will contain the converted image)
		hr = wicConverter->Initialize(wicFrame, convertToPixelFormat, WICBitmapDitherTypeErrorDiffusion, 0, 0, WICBitmapPaletteTypeCustom);
		if (FAILED(hr)) return 0;

		// this is so we know to get the image data from the wicConverter (otherwise we will get from wicFrame)
		imageConverted = true;
	}

	int bitsPerPixel = GetDXGIFormatBitsPerPixel(dxgiFormat); // number of bits per pixel
	bytesPerRow = (textureWidth * bitsPerPixel) / 8; // number of bytes in each row of the image data
	int imageSize = bytesPerRow * textureHeight; // total image size in bytes

	// allocate enough memory for the raw image data, and set imageData to point to that memory
	*imageData = (BYTE*)malloc(imageSize);

	// copy (decoded) raw image data into the newly allocated memory (imageData)
	if (imageConverted)
	{
		// if image format needed to be converted, the wic converter will contain the converted image
		hr = wicConverter->CopyPixels(0, bytesPerRow, imageSize, *imageData);
		if (FAILED(hr)) return 0;
	}
	else
	{
		// no need to convert, just copy data from the wic frame
		hr = wicFrame->CopyPixels(0, bytesPerRow, imageSize, *imageData);
		if (FAILED(hr)) return 0;
	}

	// now describe the texture with the information we have obtained from the image
	resourceDescription = {};
	resourceDescription.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDescription.Alignment = 0; // may be 0, 4KB, 64KB, or 4MB. 0 will let runtime decide between 64KB and 4MB (4MB for multi-sampled textures)
	resourceDescription.Width = textureWidth; // width of the texture
	resourceDescription.Height = textureHeight; // height of the texture
	resourceDescription.DepthOrArraySize = 1; // if 3d image, depth of 3d image. Otherwise an array of 1D or 2D textures (we only have one image, so we set 1)
	resourceDescription.MipLevels = 1; // Number of mipmaps. We are not generating mipmaps for this texture, so we have only one level
	resourceDescription.Format = dxgiFormat; // This is the dxgi format of the image (format of the pixels)
	resourceDescription.SampleDesc.Count = 1; // This is the number of samples per pixel, we just want 1 sample
	resourceDescription.SampleDesc.Quality = 0; // The quality level of the samples. Higher is better quality, but worse performance
	resourceDescription.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN; // The arrangement of the pixels. Setting to unknown lets the driver choose the most efficient one
	resourceDescription.Flags = D3D12_RESOURCE_FLAG_NONE; // no flags

	// return the size of the image. remember to delete the image once your done with it (in this tutorial once its uploaded to the gpu)
	return imageSize;
}