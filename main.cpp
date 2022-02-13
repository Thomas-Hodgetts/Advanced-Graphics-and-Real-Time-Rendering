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

	// -- Create the Device -- //

	IDXGIFactory4* dxgiFactory;
	hr = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&dxgiFactory));
	//hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	
	D3D12GetDebugInterface(IID_PPV_ARGS(&m_DebugLayer));
	m_DebugLayer->EnableDebugLayer();
	if (FAILED(hr))
	{
		return false;
	}

	IDXGIAdapter1* adapter; // adapters are the graphics card (this includes the embedded graphics on the motherboard)

	int adapterIndex = 0; // we'll start looking for directx 12  compatible graphics devices starting at index 0

	bool adapterFound = false; // set this to true when a good one was found

							   // find first hardware gpu that supports d3d 12
	while (dxgiFactory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			// we dont want a software device
			continue;
		}

		// we want a device that is compatible with direct3d 12 (feature level 11 or higher)
		hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr);
		if (SUCCEEDED(hr))
		{
			adapterFound = true;
			break;
		}

		adapterIndex++;
	}

	if (!adapterFound)
	{
		Running = false;
		return false;
	}

	// Create the device
	hr = D3D12CreateDevice(
		adapter,
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(&device)
		);
	if (FAILED(hr))
	{
		Running = false;
		return false;
	}

	// -- Create a direct command queue -- //

	D3D12_COMMAND_QUEUE_DESC cqDesc = {};
	cqDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	cqDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; // direct means the gpu can directly execute this command queue

	hr = device->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(&commandQueue)); // create the command queue
	if (FAILED(hr))
	{
		Running = false;
		return false;
	}

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

	//hr = dxgiFactory->CreateSwapChain(
	//	commandQueue, // the queue will be flushed once the swap chain is created
	//	&swapChainDesc, // give it the swap chain description we created above
	//	&tempSwapChain // store the created swap chain in a temp IDXGISwapChain interface
	//	);


	//swapChain = static_cast<IDXGISwapChain3*>(tempSwapChain);


	// -- Create the Back Buffers (render target views) Descriptor Heap -- //

	// describe an rtv descriptor heap and create
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = frameBufferCount;// + 3; // number of descriptors for this heap.
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; // this heap is a render target view heap

													   // This heap will not be directly referenced by the shaders (not shader visible), as this will store the output from the pipeline
													   // otherwise we would set the heap's flag to D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	hr = device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvDescriptorHeap));
	if (FAILED(hr))
	{
		Running = false;
		return false;
	}

	GraphicsManager gm(1920, 1080);
	OutputManager output(&gm, swapChainDesc, rtvHeapDesc);

	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;


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


	gm.CreateRootSignature(rootSignatureFlags, samples, 2, 5, 1, 4, L"Pipeline1");
	gm.CompileVertexShader(L"Pipeline1", L"VertexShader.hlsl", "main");
	gm.CompilePixelShader(L"Pipeline1", L"PixelShader.hlsl", "main");



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


	gm.AddFrameInputLayout(inputLayout, L"Pipeline1", 5);
	gm.AddFrameInputLayout(inputLayout2, L"DefaultInput2", 2);

	gm.CreatePipeline(L"Pipeline1");


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

	gm.CreateGeomerty(vertexList.data(), vertexList.size(), indexList.data(), indexList.size(), L"TestGeomerty");
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

	bool test = gm.CreateStencilDepthView(L"Default Depth Stencil", 1,&depthStencilDesc, &depthOptimizedClearValue);


	test = gm.CreateConstantBuffer(L"TestGeomerty", 1024, sizeof(ConstantBufferPerObject), 4);


	LPCWSTR filename[4] = { L"color.jpg" ,L"normals.jpg" ,L"displacement.jpg" ,L"dx12.jpg" };
	int objectCount = sizeof(filename) / sizeof(LPCWSTR);

	gm.CreateTextureHeap(filename, objectCount, L"TestGeomerty");


	gm.FlushCommandList(L"Default");

	// create the constant buffer resource heap
	// We will update the constant buffer one or more times per frame, so we will use only an upload heap
	// unlike previously we used an upload heap to upload the vertex and index data, and then copied over
	// to a default heap. If you plan to use a resource for more than a couple frames, it is usually more
	// efficient to copy to a default heap where it stays on the gpu. In this case, our constant buffer
	// will be modified and uploaded at least once per frame, so we only use an upload heap

	// first we will create a resource heap (upload heap) for each frame for the cubes constant buffers
	// As you can see, we are allocating 64KB for each resource we create. Buffer resource heaps must be
	// an alignment of 64KB. We are creating 3 resources, one for each frame. Each constant buffer is 
	// only a 4x4 matrix of floats in this tutorial. So with a float being 4 bytes, we have 
	// 16 floats in one constant buffer, and we will store 2 constant buffers in each
	// heap, one for each cube, thats only 64x2 bits, or 128 bits we are using for each
	// resource, and each resource must be at least 64KB (65536 bits)
	//for (int i = 0; i < frameBufferCount; ++i)
	//{
	//	// create resource for cube 1
	//	hr = device->CreateCommittedResource(
	//		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // this heap will be used to upload the constant buffer data
	//		D3D12_HEAP_FLAG_NONE, // no flags
	//		&CD3DX12_RESOURCE_DESC::Buffer(2048 * 64), // size of the resource heap. Must be a multiple of 64KB for single-textures and constant buffers
	//		D3D12_RESOURCE_STATE_GENERIC_READ, // will be data that is read from so we keep it in the generic read state
	//		nullptr, // we do not have use an optimized clear value for constant buffers
	//		IID_PPV_ARGS(&constantBufferUploadHeaps[i]));
	//	if (FAILED(hr))
	//	{
	//		Running = false;
	//		return false;
	//	}
	//	constantBufferUploadHeaps[i]->SetName(L"Constant Buffer Upload Resource Heap");

	//	// create resource for cube 1
	//	hr = device->CreateCommittedResource(
	//		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // this heap will be used to upload the constant buffer data
	//		D3D12_HEAP_FLAG_NONE, // no flags
	//		&CD3DX12_RESOURCE_DESC::Buffer((2048 * 64) * m_BillboardCount), // size of the resource heap. Must be a multiple of 64KB for single-textures and constant buffers
	//		D3D12_RESOURCE_STATE_GENERIC_READ, // will be data that is read from so we keep it in the generic read state
	//		nullptr, // we do not have use an optimized clear value for constant buffers
	//		IID_PPV_ARGS(&m_BillboardConstantBufferUploadHeaps[i]));
	//	if (FAILED(hr))
	//	{
	//		Running = false;
	//		return false;
	//	}
	//	m_BillboardConstantBufferUploadHeaps[i]->SetName(L"Constant Buffer Upload Resource Heap");

	//	ZeroMemory(&cbPerObject, sizeof(cbPerObject));

	//	CD3DX12_RANGE readRange(0, 0);	// We do not intend to read from this resource on the CPU. (so end is less than or equal to begin)
	//	
	//	// map the resource heap to get a gpu virtual address to the beginning of the heap
	//	hr = constantBufferUploadHeaps[i]->Map(0, &readRange, reinterpret_cast<void**>(&cbvGPUAddress[i]));
	//	hr = m_BillboardConstantBufferUploadHeaps[i]->Map(0, &readRange, reinterpret_cast<void**>(&m_BillboardGPUAddress[i]));

	//	// Because of the constant read alignment requirements, constant buffer views must be 256 bit aligned. Our buffers are smaller than 256 bits,
	//	// so we need to add spacing between the two buffers, so that the second buffer starts at 256 bits from the beginning of the resource heap.
	//	memcpy(cbvGPUAddress[i], &cbPerObject, sizeof(cbPerObject)); // cube1's constant buffer data
	//	memcpy(cbvGPUAddress[i] + ConstantBufferPerObjectAlignedSize, &cbPerObject, sizeof(cbPerObject)); // cube2's constant buffer data
	//	memcpy(cbvGPUAddress[i] + ConstantBufferPerObjectAlignedSize + ConstantBufferPerObjectAlignedSize, &cbPerObject, sizeof(cbPerObject)); // cube3s constant buffer data

	//	if (m_ShadowMapping)
	//	{
	//		memcpy(cbvGPUAddress[i] + ConstantBufferPerObjectAlignedSize + ConstantBufferPerObjectAlignedSize + ConstantBufferPerObjectAlignedSize, &cbPerObject, sizeof(cbPerObject)); // cube3s constant buffer data
	//		memcpy(cbvGPUAddress[i] + ConstantBufferPerObjectAlignedSize + ConstantBufferPerObjectAlignedSize + ConstantBufferPerObjectAlignedSize + ConstantBufferPerObjectAlignedSize, &cbPerObject, sizeof(cbPerObject)); // cube3s constant buffer data
	//	}

	//	for (size_t j = 0; j < m_BillboardCount; j++)
	//	{
	//		memcpy(m_BillboardGPUAddress[i] + (ConstantBufferPerObjectAlignedSize * j), &cbPerObject, sizeof(cbPerObject));
	//	}
	//}

	//// load the image, create a texture resource and descriptor heap

	//// create the descriptor heap that will store our srv
 //   D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	//heapDesc.NumDescriptors = 5;
	//heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	//heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	//hr = device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&mainDescriptorHeap));
	//if (FAILED(hr))
	//{
	//	Running = false;
	//}


	//buffOffset = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);



	//// Load the image from file
	//D3D12_RESOURCE_DESC textureDesc;
	//int imageBytesPerRow;

	//LPCWSTR filename[4] = { L"color.jpg" ,L"normals.jpg" ,L"displacement.jpg" ,L"dx12.jpg" };
	//int objectCount = sizeof(filename) / sizeof(LPCWSTR);

	//gm.CreateTextureHeap(filename, objectCount, L"TestGeomerty");

	//CD3DX12_CPU_DESCRIPTOR_HANDLE hdescriptor(mainDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	//CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	//for (size_t i = 0; i < objectCount; i++)
	//{
	//	int imageSize = LoadImageDataFromFile(&imageData, textureDesc, filename[i], imageBytesPerRow);
	//	// make sure we have data
	//	if (imageSize <= 0)
	//	{
	//		Running = false;
	//		return false;
	//	}

	//	// create a default heap where the upload heap will copy its contents into (contents being the texture)
	//	hr = device->CreateCommittedResource(
	//		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // a default heap
	//		D3D12_HEAP_FLAG_NONE, // no flags
	//		&textureDesc, // the description of our texture
	//		D3D12_RESOURCE_STATE_COPY_DEST, // We will copy the texture from the upload heap to here, so we start it out in a copy dest state
	//		nullptr, // used for render targets and depth/stencil buffers
	//		IID_PPV_ARGS(&textureBuffer[i]));
	//	if (FAILED(hr))
	//	{
	//		Running = false;
	//		return false;
	//	}
	//	textureBuffer[i]->SetName(L"Texture Buffer Resource Heap");

	//	UINT64 textureUploadBufferSize;
	//	device->GetCopyableFootprints(&textureDesc, 0, 1, 0, nullptr, nullptr, nullptr, &textureUploadBufferSize);

	//	// now we create an upload heap to upload our texture to the GPU
	//	hr = device->CreateCommittedResource(
	//		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // upload heap
	//		D3D12_HEAP_FLAG_NONE, // no flags
	//		&CD3DX12_RESOURCE_DESC::Buffer(textureUploadBufferSize), // resource description for a buffer (storing the image data in this heap just to copy to the default heap)
	//		D3D12_RESOURCE_STATE_GENERIC_READ, // We will copy the contents from this heap to the default heap above
	//		nullptr,
	//		IID_PPV_ARGS(&textureBufferUploadHeap[i]));
	//	if (FAILED(hr))
	//	{
	//		Running = false;
	//		return false;
	//	}
	//	textureBufferUploadHeap[i]->SetName(L"Texture Buffer Upload Resource Heap");

	//	// store vertex buffer in upload heap
	//	D3D12_SUBRESOURCE_DATA textureData = {};
	//	textureData.pData = &imageData[0]; // pointer to our image data
	//	textureData.RowPitch = imageBytesPerRow; // size of all our triangle vertex data
	//	textureData.SlicePitch = imageBytesPerRow * textureDesc.Height; // also the size of our triangle vertex data
	//	// Now we copy the upload buffer contents to the default heap
	//	UpdateSubresources(commandList, textureBuffer[i], textureBufferUploadHeap[i], 0, 0, 1, &textureData);
	//	// transition the texture default heap to a pixel shader resource (we will be sampling from this heap in the pixel shader to get the color of pixels)
	//	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(textureBuffer[i], D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

	//	//BOOK

	//	// now we create a shader resource view (descriptor that points to the texture and describes it)
	//	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	//	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	//	srvDesc.Format = textureDesc.Format;
	//	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	//	srvDesc.Texture2D.MipLevels = 1;

	//	device->CreateShaderResourceView(textureBuffer[i], &srvDesc, hdescriptor);
	//	hdescriptor.Offset(1, buffOffset);

	//	if (m_TextureSetUp && i == 3)
	//	{

	//		D3D12_RESOURCE_DESC S;
	//		textureDesc = {};
	//		textureDesc.MipLevels = 1;
	//		textureDesc.Height = Height;
	//		textureDesc.Width = Width;
	//		textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	//		textureDesc.DepthOrArraySize = 1;
	//		textureDesc.SampleDesc.Count = 1;
	//		textureDesc.SampleDesc.Quality = 0;
	//		textureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	//		textureDesc.Alignment = 0;
	//		textureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	//		textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;


	//		 //create a default heap where the upload heap will copy its contents into (contents being the texture)
	//		hr = device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES, &textureDesc, D3D12_RESOURCE_STATE_RENDER_TARGET, nullptr, IID_PPV_ARGS(&textureBuffer[4]));
	//		if (FAILED(hr))
	//		{
	//			hr = device->GetDeviceRemovedReason();
	//			Running = false;
	//			return false;
	//		}
	//		textureBuffer[4]->SetName(L"Texture Buffer Resource Heap");

	//		UINT64 textureUploadBufferSize;
	//		device->GetCopyableFootprints(&textureDesc, 0, 1, 0, nullptr, nullptr, nullptr, &textureUploadBufferSize);

	//		// now we create an upload heap to upload our texture to the GPU
	//		hr = device->CreateCommittedResource(
	//			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // upload heap
	//			D3D12_HEAP_FLAG_NONE, // no flags
	//			&CD3DX12_RESOURCE_DESC::Buffer(textureUploadBufferSize), // resource description for a buffer (storing the image data in this heap just to copy to the default heap)
	//			D3D12_RESOURCE_STATE_GENERIC_READ, // We will copy the contents from this heap to the default heap above
	//			nullptr,
	//			IID_PPV_ARGS(&textureBufferUploadHeap[4]));
	//		if (FAILED(hr))
	//		{
	//			Running = false;
	//			return false;
	//		}
	//		textureBufferUploadHeap[4]->SetName(L"Texture Buffer Upload Resource Heap");

	//		//BOOK

	//		// now we create a shader resource view (descriptor that points to the texture and describes it)
	//		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	//		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	//		srvDesc.Format = textureDesc.Format;
	//		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
	//		srvDesc.Texture2D.MipLevels = 1;

	//		D3D12_SUBRESOURCE_DATA textureData = {};
	//		textureData.pData = &imageData[0]; // pointer to our image data
	//		textureData.RowPitch = imageBytesPerRow; // size of all our triangle vertex data
	//		textureData.SlicePitch = imageBytesPerRow * textureDesc.Height; // also the size of our triangle vertex data
	//		// Now we copy the upload buffer contents to the default heap
	//		UpdateSubresources(commandList, textureBuffer[i], textureBufferUploadHeap[i], 0, 0, 1, &textureData);
	//		// transition the texture default heap to a pixel shader resource (we will be sampling from this heap in the pixel shader to get the color of pixels)
	//		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(textureBuffer[i], D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

	//		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	//		rtvHandle.Offset(3, rtvDescriptorSize);

	//		device->CreateRenderTargetView(textureBuffer[4], nullptr, rtvHandle);
	//		rtvHandle.Offset(1, rtvDescriptorSize);

	//		device->CreateShaderResourceView(textureBuffer[4], &srvDesc, hdescriptor);
	//		hdescriptor.Offset(1, buffOffset);


	//		// Create the MSAA depth/stencil buffer.
	//		auto depthStencilDesc = CD3DX12_RESOURCE_DESC::Tex2D(
	//			DXGI_FORMAT_D32_FLOAT,
	//			static_cast<UINT>(Width),
	//			static_cast<UINT>(Height),
	//			1,
	//			1,
	//			4,
	//			0
	//		);
	//		depthStencilDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	//		D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
	//		depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	//		depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
	//		depthOptimizedClearValue.DepthStencil.Stencil = 0;

	//		hr = device->CreateCommittedResource(
	//			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
	//			D3D12_HEAP_FLAG_NONE,
	//			&depthStencilDesc,
	//			D3D12_RESOURCE_STATE_DEPTH_WRITE,
	//			&depthOptimizedClearValue,
	//			IID_PPV_ARGS(&depthStencilBuffer2));

	//		if (FAILED(hr))
	//		{
	//			Running = false;
	//			return false;
	//		}

	//		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	//		dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	//		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;

	//		dsvHandle.Offset(1, dsvDescriptorSize);

	//		device->CreateDepthStencilView(depthStencilBuffer2, &dsvDesc, dsvHandle);

	//		auto msaaRTDesc = CD3DX12_RESOURCE_DESC::Tex2D(
	//			DXGI_FORMAT_R8G8B8A8_UNORM,
	//			static_cast<UINT>(Width),
	//			static_cast<UINT>(Height),
	//			1, // This render target view has only one texture.
	//			1, // Use a single mipmap level
	//			4,
	//			0
	//		);
	//		msaaRTDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	//		msaaRTDesc.SampleDesc.Count = 4;
	//		msaaRTDesc.SampleDesc.Quality = 0;


	//		FLOAT f[4] = { 0,0,0,0 };
	//		D3D12_CLEAR_VALUE msaaOptimizedClearValue = {};
	//		msaaOptimizedClearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	//		memcpy(msaaOptimizedClearValue.Color, f, sizeof(float) * 4);

	//		hr = device->CreateCommittedResource(
	//			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
	//			D3D12_HEAP_FLAG_NONE,
	//			&msaaRTDesc,
	//			D3D12_RESOURCE_STATE_RESOLVE_SOURCE,
	//			&msaaOptimizedClearValue,
	//			IID_PPV_ARGS(&MSAA_RenderTarget));

	//		device->CreateRenderTargetView(MSAA_RenderTarget, nullptr, rtvHandle);

	//		svQuad[0].pos = XMFLOAT3(-1.0f, 1.0f, 0.0f);
	//		svQuad[0].tex = XMFLOAT2(0.0f, 0.0f);

	//		svQuad[1].pos = XMFLOAT3(1.0f, 1.0f, 0.0f);
	//		svQuad[1].tex = XMFLOAT2(1.0f, 0.0f);

	//		svQuad[2].pos = XMFLOAT3(-1.0f, -1.0f, 0.0f);
	//		svQuad[2].tex = XMFLOAT2(0.0f, 1.0f);

	//		svQuad[3].pos = XMFLOAT3(1.0f, -1.0f, 0.0f);
	//		svQuad[3].tex = XMFLOAT2(1.0f, 1.0f);


	//		m_TextureSetUp = false;

	//	}

	//}
	//// create a vertex buffer view for the triangle. We get the GPU memory address to the vertex pointer using the GetGPUVirtualAddress() method
	//vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	//vertexBufferView.StrideInBytes = sizeof(Vertex);
	//vertexBufferView.SizeInBytes = vBufferSize;

	//// create a vertex buffer view for the triangle. We get the GPU memory address to the vertex pointer using the GetGPUVirtualAddress() method
	//indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
	//indexBufferView.Format = DXGI_FORMAT_R32_UINT; // 32-bit unsigned integer (this is what a dword is, double word, a word is 2 bytes)
	//indexBufferView.SizeInBytes = iBufferSize;

	//m_BillboardVertexView = std::vector<D3D12_VERTEX_BUFFER_VIEW>(m_BillboardCount);
	//m_BillboardIndexView = std::vector<D3D12_INDEX_BUFFER_VIEW>(m_BillboardCount);

	//if (m_RenderToTexture)
	//{
	//	m_ScreenQuadVertexView.BufferLocation = m_ScreenQuadVertexBuffer->GetGPUVirtualAddress();
	//	m_ScreenQuadVertexView.StrideInBytes = sizeof(Vertex);
	//	m_ScreenQuadVertexView.SizeInBytes = vBufferSize2;
	//	m_ScreenQuadIndexView.BufferLocation = m_ScreenQuadIndexBuffer->GetGPUVirtualAddress();
	//	m_ScreenQuadIndexView.Format = DXGI_FORMAT_R32_UINT; // 32-bit unsigned integer (this is what a dword is, double word, a word is 2 bytes)
	//	m_ScreenQuadIndexView.SizeInBytes = iBufferSize2;
	//}

	//for (size_t i = m_BillboardCount; i--;)
	//{

	//	// create a vertex buffer view for the triangle. We get the GPU memory address to the vertex pointer using the GetGPUVirtualAddress() method
	//	m_BillboardVertexView[i].BufferLocation = m_BillboardVertex[i]->GetGPUVirtualAddress();
	//	m_BillboardVertexView[i].StrideInBytes = sizeof(Vertex);
	//	m_BillboardVertexView[i].SizeInBytes = vBufferSize2;

	//	// create a vertex buffer view for the triangle. We get the GPU memory address to the vertex pointer using the GetGPUVirtualAddress() method
	//	m_BillboardIndexView[i].BufferLocation = m_BillboardIndex[i]->GetGPUVirtualAddress();
	//	m_BillboardIndexView[i].Format = DXGI_FORMAT_R32_UINT; // 32-bit unsigned integer (this is what a dword is, double word, a word is 2 bytes)
	//	m_BillboardIndexView[i].SizeInBytes = iBufferSize2;
	//}




	//CAMERA AND OBJECT SET UP

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
	cameraPosition = XMFLOAT4(0.0f, 0.0f, -6.0f, 0.0f);
	cameraTarget = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	cameraUp = XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f);

	//// set starting cubes position
	//// first cube
	//cube1Position = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f); // set cube 1's position
	//XMVECTOR posVec = XMLoadFloat4(&cube1Position); // create xmvector for cube1's position

	//tmpMat = XMMatrixTranslationFromVector(posVec); // create translation matrix from cube1's position vector
	//XMStoreFloat4x4(&cube1RotMat, XMMatrixIdentity()); // initialize cube1's rotation matrix to identity matrix
	//XMStoreFloat4x4(&cube1WorldMat, tmpMat); // store cube1's world matrix

	//// second cube
	//cube2PositionOffset = XMFLOAT4(1.5f, 0.0f, 0.0f, 0.0f);
	//posVec = XMLoadFloat4(&cube2PositionOffset) + XMLoadFloat4(&cube1Position); // create xmvector for cube2's position
	//																			// we are rotating around cube1 here, so add cube2's position to cube1

	//tmpMat = XMMatrixTranslationFromVector(posVec); // create translation matrix from cube2's position offset vector
	//XMStoreFloat4x4(&cube2RotMat, XMMatrixIdentity()); // initialize cube2's rotation matrix to identity matrix
	//XMStoreFloat4x4(&cube2WorldMat, tmpMat); // store cube2's world matrix

	m_Manager = new SystemManager();
	m_Manager->SetUpCamera(cameraPosition, cameraTarget, cameraUp, Width, Height, 0.1, 1000);
	m_Manager->ReturnCamera()->Update();

	//Transform* transform = new Transform(Vector3D(0.0f, 0.0f, 0.0f), Vector3D(0.0f, 0.0f, 0.0f), Vector3D(2.0f, 2.0f, 2.0f));
	//XMStoreFloat4x4(&transform->RotationalMatrix, XMMatrixIdentity());
	//CreateObjectStruct COS(device, commandList, "Obj1", ObjectType::GameObj, vertexList, indexList, 1, ConstantBufferPerObjectAlignedSize, transform);
	//m_Manager->BuildObject(COS);

	//transform = new Transform(Vector3D(1.5f, 0.0f, 0.0f), Vector3D(0.0f, 00.0f, 0.0f), Vector3D(0.5f, 0.5f, 0.5f));
	//XMStoreFloat4x4(&transform->RotationalMatrix, XMMatrixIdentity());
	//COS = CreateObjectStruct(device, commandList, "Obj2", ObjectType::GameObj, vertexList, indexList, 2, ConstantBufferPerObjectAlignedSize, transform);
	//m_Manager->BuildObject(COS);


	////Thing to note: Constant buffer aligned size didn't increase on billboard creation
	//for (size_t i = m_BillboardCount; i--;)
	//{
	//	float x = GenerateRandomNumber(0,10);
	//	float y = GenerateRandomNumber(0,10);
	//	float z = GenerateRandomNumber(5,15);
	//	if (GenerateRandomNumber(1,2) % 2)
	//	{
	//		x = -x;
	//	}
	//	if (GenerateRandomNumber(1, 2) % 2)
	//	{
	//		y = -y;
	//	}

	//	transform = new Transform(Vector3D(x, y, z), Vector3D(0.0f, 0.0f, 0.0f), Vector3D(1.0f, 1.0f, 1.0f));
	//	XMStoreFloat4x4(&transform->RotationalMatrix, XMMatrixIdentity());
	//	COS = CreateObjectStruct(device, commandList, std::to_string(i), ObjectType::GameObj, verts2, indies2, 3 + i, ConstantBufferPerObjectAlignedSize, transform);
	//	m_Manager->BuildObject(COS);

	//}

	shinyMaterial.AmbientMtrl = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	shinyMaterial.DiffuseMtrl = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	shinyMaterial.SpecularMtrl = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);

	// Setup the scene's light
	basicLight.AmbientLight = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	basicLight.DiffuseLight = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	basicLight.SpecularLight = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	basicLight.SpecularPower = 32.0f;
	basicLight.LightVecW = XMFLOAT3(0.0f, 0.0f, -6);

	XMMATRIX scale = XMMatrixScaling(1,1,1);
	XMMATRIX rotation = XMMatrixRotationX(0) * XMMatrixRotationY(0) * XMMatrixRotationZ(0);
	XMMATRIX translation = XMMatrixTranslation(basicLight.LightVecW.x, basicLight.LightVecW.y, basicLight.LightVecW.z);

	XMStoreFloat4x4(&m_LightMatrix, scale * rotation * translation);

	if (m_ShadowMapping)
	{
		//m_SM = ShadowMap(device, 2048, 2048);
		//m_BS.Center = { 0.f, 0.f, 0.f };
		//m_BS.Radius = sqrtf(10.f * 10.f + 15.f * 15.f);

		//dsvHandle.Offset(1, dsvDescriptorSize);

		//m_SM.BuildDescriptors(
		//	hdescriptor,
		//	CD3DX12_GPU_DESCRIPTOR_HANDLE(mainDescriptorHeap->GetGPUDescriptorHandleForHeapStart(), 0, srvDescriptorSize),
		//	dsvHandle);
	}

	// Now we execute the command list to upload the initial assets (triangle data)
	//commandList->Close();
	//ID3D12CommandList* ppCommandLists[] = { commandList };
	//commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	//// increment the fence value now, otherwise the buffer might not be uploaded by the time we start drawing
	//fenceValue[frameIndex]++;
	//hr = commandQueue->Signal(fence[frameIndex], fenceValue[frameIndex]);
	//if (FAILED(hr))
	//{
	//	Running = false;
	//	return false;
	//}

	//// we are done with image data now that we've uploaded it to the gpu, so free it up
	//delete[] imageData;

	m_GameManager = gm;

	return true;
}

void Update()
{

	//GameObject* GO = dynamic_cast<GameObject*>(m_Manager->GetStoredObject(0));
	//GameObject* GO2 = dynamic_cast<GameObject*>(m_Manager->GetStoredObject(1));
	//GameObject* GO3 = dynamic_cast<GameObject*>(m_Manager->GetStoredObject(2));


	m_Time++;
	if (GetAsyncKeyState('W') & 0x8000)
		m_Manager->ReturnCamera()->Walk(0.005f);

	if (GetAsyncKeyState('S') & 0x8000)
		m_Manager->ReturnCamera()->Walk(-0.005f);

	if (GetAsyncKeyState('A') & 0x8000)
		m_Manager->ReturnCamera()->Strafe(-0.005f);

	if (GetAsyncKeyState('D') & 0x8000)
		m_Manager->ReturnCamera()->Strafe(0.005f);
	if (GetAsyncKeyState('E') & 0x8000)
	{
		//Vector3D rot = GO->m_Transform->ReturnRot();
		//GO->m_Transform->SetRot(Vector3D(rot.ReturnX(), rot.ReturnY() + 0.005f, rot.ReturnZ()));

		//rot = GO2->m_Transform->ReturnRot();
		//GO2->m_Transform->SetRot(Vector3D(rot.ReturnX() + 0.0003f, rot.ReturnY() + 0.000420f, rot.ReturnZ() + 0.00069f));
	}

	if (GetAsyncKeyState('Q') & 0x8000)
	{
		/*Vector3D rot = GO->m_Transform->ReturnRot();
		GO->m_Transform->SetRot(Vector3D(rot.ReturnX(), rot.ReturnY() - 0.00005f, rot.ReturnZ()));

		rot = GO2->m_Transform->ReturnRot();
		GO2->m_Transform->SetRot(Vector3D(rot.ReturnX() - 0.0003f, rot.ReturnY() - 0.000420f, rot.ReturnZ() - 0.00069f));*/
	}

	if (GetAsyncKeyState(0x26) & 0x8000)
		basicLight.LightVecW.z = basicLight.LightVecW.z + 0.0005f;

	if (GetAsyncKeyState(0x28) & 0x8000)
		basicLight.LightVecW.z = basicLight.LightVecW.z + -0.0005f;

	if (GetAsyncKeyState(0x25) & 0x8000)
		basicLight.LightVecW.x = basicLight.LightVecW.x + -0.0005f;

	if (GetAsyncKeyState(0x27) & 0x8000)
		basicLight.LightVecW.x = basicLight.LightVecW.x + 0.0005f;

	if (GetAsyncKeyState('G') & 0x8000)
		m_Rotate = -m_Rotate;


	if (m_Rotate)
	{
		/*Vector3D rot = GO->m_Transform->ReturnRot();
		GO->m_Transform->SetRot(Vector3D(rot.ReturnX(), rot.ReturnY() + 0.00005f, rot.ReturnZ()));

		rot = GO2->m_Transform->ReturnRot();
		GO2->m_Transform->SetRot(Vector3D(rot.ReturnX() + 0.0003f, rot.ReturnY() + 0.000420f, rot.ReturnZ() + 0.00069f));*/
	}


	//GO->Update(0.f);
	//GO2->Update(0.f);
	//GO3->Update(0.f);

	// update constant buffer for cube1
	// create the wvp matrix and store in constant buffer
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

	cbPerObject.point = basicLight;
	cbPerObject.Mat = shinyMaterial;
	cbPerObject.projection = m_Manager->ReturnCamera()->ReturnViewPlusProjection().m_projection;

	//XMMATRIX wvpMat = GO->m_Particle->ReturnWorldMatrix() * viewMat * projMat; // create wvp matrix
	//XMMATRIX transposed = XMMatrixTranspose(wvpMat); // must transpose wvp matrix for the gpu
	//XMStoreFloat4x4(&cbPerObject.wvpMat, transposed); // store transposed wvp matrix in constant buffer
	//XMStoreFloat4x4(&cbPerObject.worldPos,GO->m_Particle->ReturnWorldMatrix());

	m_GameManager.UpdateObjectConstantBuffer(cbPerObject, L"TestGeomerty", 0);


	// copy our ConstantBuffer instance to the mapped constant buffer resource
	//memcpy(cbvGPUAddress[frameIndex], &cbPerObject, sizeof(cbPerObject));


	////wvpMat = GO2->m_Particle->ReturnWorldMatrix() * viewMat * projMat; // create wvp matrix
	//transposed = XMMatrixTranspose(wvpMat); // must transpose wvp matrix for the gpu
	//XMStoreFloat4x4(&cbPerObject.wvpMat, transposed); // store transposed wvp matrix in constant buffer
	//XMStoreFloat4x4(&cbPerObject.worldPos, GO2->m_Particle->ReturnWorldMatrix());

	// copy our ConstantBuffer instance to the mapped constant buffer resource
	//memcpy(cbvGPUAddress[frameIndex] + ConstantBufferPerObjectAlignedSize, &cbPerObject, sizeof(cbPerObject));
	m_GameManager.UpdateObjectConstantBuffer(cbPerObject, L"TestGeomerty", 1);

	//if (m_ShadowMapping)
	//{
	//	cbShadow = cbPerObject;
	//	XMFLOAT3 dir(0,0,1);
	//	XMVECTOR lightDir = XMLoadFloat3(&dir);
	//	XMVECTOR lightPos = XMLoadFloat3(&dir) *-2.0 * m_BS.Radius;
	//	//XMVECTOR lightPos = -2.0 * m_BS.Radius * lightDir;
	//	XMVECTOR targetPos = XMLoadFloat3(&m_BS.Center);
	//	XMVECTOR lightUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	//	XMMATRIX lightView = XMMatrixLookAtLH(lightPos, targetPos, lightUp);

	//	// Transform bounding sphere to light space.
	//	XMFLOAT3 sphereCenterLS;
	//	XMStoreFloat3(&sphereCenterLS, XMVector3TransformCoord(targetPos, lightView));

	//	// Ortho frustum in light space encloses scene.
	//	float l = sphereCenterLS.x - m_BS.Radius;
	//	float b = sphereCenterLS.y - m_BS.Radius;
	//	float n = sphereCenterLS.z - m_BS.Radius;
	//	float r = sphereCenterLS.x + m_BS.Radius;
	//	float t = sphereCenterLS.y + m_BS.Radius;
	//	float f = sphereCenterLS.z + m_BS.Radius;


	//	XMMATRIX lightProj = XMMatrixOrthographicOffCenterLH(l,r,b,t,n,f);
	//	XMMATRIX T(
	//		0.5f, 0.f, 0.f, 0.f,
	//		0.f, -0.5f, 0.f, 0.f,
	//		0.f, 0.f, 1.f, 0.f,
	//		0.5f, 0.5f, 0.f, 1.0f
	//	);

	//	XMMATRIX S = lightView * lightProj * T;

	//	XMStoreFloat4x4(&m_ShadowTransform, S);
	//	XMStoreFloat4x4(&m_LightProj, lightProj);
	//	XMStoreFloat4x4(&m_LightView, lightView);

	//	wvpMat = GO3->m_Particle->ReturnWorldMatrix() * viewMat * projMat; // create wvp matrix
	//	transposed = XMMatrixTranspose(wvpMat); // must transpose wvp matrix for the gpu
	//	XMStoreFloat4x4(&cbPerObject.wvpMat, transposed); // store transposed wvp matrix in constant buffer
	//	cbPerObject.projection = m_Manager->ReturnCamera()->ReturnViewPlusProjection().m_projection;
	//	XMStoreFloat4x4(&cbPerObject.worldPos, GO3->m_Particle->ReturnWorldMatrix());
	//	cbPerObject.shadowTransform = m_ShadowTransform;
	//	cbPerObject.mode = 3;
	//	cbPerObject.point = basicLight;
	//	cbPerObject.Mat = shinyMaterial;
	//	memcpy(cbvGPUAddress[frameIndex] + ConstantBufferPerObjectAlignedSize + ConstantBufferPerObjectAlignedSize, &cbPerObject, sizeof(cbPerObject));


	//	cbShadow.EyePosW = basicLight.LightVecW;
	//	cbShadow.shadowTransform = m_ShadowTransform;
	//	cbShadow.Mat = shinyMaterial;
	//	cbShadow.point = basicLight;


	//	XMMATRIX wvpMat = GO->m_Particle->ReturnWorldMatrix() * lightView * lightProj; // create wvp matrix
	//	XMMATRIX transposed = XMMatrixTranspose(wvpMat); // must transpose wvp matrix for the gpu
	//	XMStoreFloat4x4(&cbShadow.wvpMat, transposed); // store transposed wvp matrix in constant buffer
	//	cbShadow.projection = m_LightProj;
	//	XMStoreFloat4x4(&cbShadow.worldPos, GO->m_Particle->ReturnWorldMatrix());

	//	// copy our ConstantBuffer instance to the mapped constant buffer resource
	//	memcpy(cbvGPUAddress[frameIndex] + (ConstantBufferPerObjectAlignedSize * 3), &cbShadow, sizeof(cbShadow));

	//	wvpMat = GO2->m_Particle->ReturnWorldMatrix() * lightView * lightProj;  // create wvp matrix
	//	transposed = XMMatrixTranspose(wvpMat); // must transpose wvp matrix for the gpu
	//	XMStoreFloat4x4(&cbShadow.wvpMat, transposed); // store transposed wvp matrix in constant buffer
	//	cbShadow.projection = m_LightProj;
	//	XMStoreFloat4x4(&cbShadow.worldPos, GO2->m_Particle->ReturnWorldMatrix());

	//	// copy our ConstantBuffer instance to the mapped constant buffer resource
	//	memcpy(cbvGPUAddress[frameIndex] + (ConstantBufferPerObjectAlignedSize * 4), &cbShadow, sizeof(cbShadow));
	//}
	//else
	//{
	//	for (size_t i = m_BillboardCount; i--;)
	//	{
	//		GameObject* GO4 = dynamic_cast<GameObject*>(m_Manager->GetStoredObject(i + 3));
	//		GO4->Update(0.f);
	//		XMMATRIX wvpMat = GO4->m_Particle->ReturnWorldMatrix() * viewMat * projMat; // create wvp matrix
	//		cbPerObject.mode = 1;
	//		XMMATRIX transposed = XMMatrixTranspose(wvpMat); // must transpose wvp matrix for the gpu
	//		XMStoreFloat4x4(&cbPerObject.wvpMat, transposed); // store transposed wvp matrix in constant buffer
	//		XMStoreFloat4x4(&cbPerObject.worldPos, GO4->m_Particle->ReturnWorldMatrix());
	//		memcpy(m_BillboardGPUAddress[frameIndex] + (ConstantBufferPerObjectAlignedSize * i), &cbPerObject, sizeof(cbPerObject));
	//	}
	//}

	int i = 0;

}

void UpdatePipeline()
{
	Geometry GO1 = dynamic_cast<GameObject*>(m_Manager->GetStoredObject(0))->GetGeometryData();
	Geometry GO2 = dynamic_cast<GameObject*>(m_Manager->GetStoredObject(1))->GetGeometryData();
	Geometry GO3 = dynamic_cast<GameObject*>(m_Manager->GetStoredObject(2))->GetGeometryData();

	

	HRESULT hr;

	// We have to wait for the gpu to finish with the command allocator before we reset it
	WaitForPreviousFrame();

	// we can only reset an allocator once the gpu is done with it
	// resetting an allocator frees the memory that the command list was stored in
	hr = commandAllocator[frameIndex]->Reset();
	if (FAILED(hr))
	{
		Running = false;
	}



	m_DrawObjectStructs.frameIndex = frameIndex;
	m_DrawObjectStructs.commandAllocator = commandAllocator[frameIndex];
	m_DrawObjectStructs.renderTargets = renderTargets[frameIndex];


	
	// reset the command list. by resetting the command list we are putting it into
	// a recording state so we can start recording commands into the command allocator.
	// the command allocator that we reference here may have multiple command lists
	// associated with it, but only one can be recording at any time. Make sure
	// that any other command lists associated to this command allocator are in
	// the closed state (not recording).
	// Here you will pass an initial pipeline state object as the second parameter,
	// but in this tutorial we are only clearing the rtv, and do not actually need
	// anything but an initial default pipeline, which is what we get by setting
	// the second parameter to NULL
	hr = commandList->Reset(commandAllocator[frameIndex], pipelineStateObject);
	if (FAILED(hr))
	{
		Running = false;
	}

	// here we start recording commands into the commandList (which all the commands will be stored in the commandAllocator)

	// transition the "frameIndex" render target from the present state to the render target state so the command list draws to it starting from here
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	// here we again get the handle to our current render target view so we can set it as the render target in the output merger stage of the pipeline
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), frameIndex, rtvDescriptorSize);

	// get a handle to the depth/stencil buffer
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	// set the render target for the output merger stage (the output of the pipeline)
	commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	// Clear the render target by using the ClearRenderTargetView command
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	// clear the depth/stencil buffer
	commandList->ClearDepthStencilView(dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	// set root signature
	commandList->SetGraphicsRootSignature(rootSignature); // set the root signature

	// set the descriptor heap
	ID3D12DescriptorHeap* descriptorHeaps[] = { mainDescriptorHeap };
	commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	// set the descriptor table to the descriptor heap (parameter 1, as constant buffer root descriptor is parameter index 0)
	commandList->SetGraphicsRootDescriptorTable(1, mainDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

	commandList->RSSetViewports(1, &viewport); // set the viewports
	commandList->RSSetScissorRects(1, &scissorRect); // set the scissor rects
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // set the primitive topology
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView); // set the vertex buffer (using the vertex buffer view)
	commandList->IASetIndexBuffer(&indexBufferView);

	// first cube

	//Book


	if (m_ShadowMapping)
	{
		commandList->SetGraphicsRootConstantBufferView(0, constantBufferUploadHeaps[frameIndex]->GetGPUVirtualAddress());
		CD3DX12_GPU_DESCRIPTOR_HANDLE Cube1tex(mainDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		commandList->SetGraphicsRootDescriptorTable(1, Cube1tex);
		Cube1tex.Offset(2, buffOffset);
		commandList->SetGraphicsRootDescriptorTable(2, Cube1tex);
		Cube1tex.Offset(2, buffOffset);
		commandList->SetGraphicsRootDescriptorTable(3, Cube1tex);

		commandList->DrawIndexedInstanced(numCubeIndices, 1, 0, 0, 0);

		commandList->SetGraphicsRootConstantBufferView(0, constantBufferUploadHeaps[frameIndex]->GetGPUVirtualAddress() + ConstantBufferPerObjectAlignedSize);

		//Book
		CD3DX12_GPU_DESCRIPTOR_HANDLE Cube2tex(mainDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		Cube2tex.Offset(4, buffOffset);
		commandList->SetGraphicsRootDescriptorTable(1, Cube2tex);
		Cube2tex.Offset(0, buffOffset);
		commandList->SetGraphicsRootDescriptorTable(2, Cube2tex);
		Cube2tex.Offset(0, buffOffset);
		commandList->SetGraphicsRootDescriptorTable(3, Cube2tex);

		// draw second cube
		commandList->DrawIndexedInstanced(numCubeIndices, 1, 0, 0, 0);

		commandList->SetGraphicsRootConstantBufferView(0, constantBufferUploadHeaps[frameIndex]->GetGPUVirtualAddress() + ConstantBufferPerObjectAlignedSize + ConstantBufferPerObjectAlignedSize);
		//Book
		CD3DX12_GPU_DESCRIPTOR_HANDLE Cube3tex(mainDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		Cube3tex.Offset(3, buffOffset);
		commandList->SetGraphicsRootDescriptorTable(1, Cube3tex);
		Cube3tex.Offset(0, buffOffset);
		commandList->SetGraphicsRootDescriptorTable(2, Cube3tex);
		Cube3tex.Offset(1, buffOffset);
		commandList->SetGraphicsRootDescriptorTable(4, Cube3tex);
		commandList->DrawIndexedInstanced(numCubeIndices, 1, 0, 0, 0);
	}

	else if (m_RenderToTexture)
	{
		commandList->SetGraphicsRootConstantBufferView(0, constantBufferUploadHeaps[frameIndex]->GetGPUVirtualAddress());
		CD3DX12_GPU_DESCRIPTOR_HANDLE Cube1tex(mainDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		commandList->SetGraphicsRootDescriptorTable(1, Cube1tex);
		Cube1tex.Offset(2, buffOffset);
		commandList->SetGraphicsRootDescriptorTable(2, Cube1tex);
		Cube1tex.Offset(2, buffOffset);
		commandList->SetGraphicsRootDescriptorTable(3, Cube1tex);

		commandList->DrawIndexedInstanced(numCubeIndices, 1, 0, 0, 0);


		commandList->SetGraphicsRootConstantBufferView(0, constantBufferUploadHeaps[frameIndex]->GetGPUVirtualAddress() + ConstantBufferPerObjectAlignedSize);

		commandList->IASetVertexBuffers(0, 1, &m_ScreenQuadVertexView); // set the vertex buffer (using the vertex buffer view)
		commandList->IASetIndexBuffer(&m_ScreenQuadIndexView);


		commandList->SetPipelineState(pipelineStateObject2);

		CD3DX12_GPU_DESCRIPTOR_HANDLE Cube2tex(mainDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		Cube2tex.Offset(4, buffOffset);
		commandList->SetGraphicsRootDescriptorTable(1, Cube2tex);

		// draw second cube
		commandList->DrawIndexedInstanced(numCubeIndices2, 1, 0, 0, 0);

		commandList->SetPipelineState(pipelineStateObject);

		commandList->IASetVertexBuffers(0, 1, &vertexBufferView); // set the vertex buffer (using the vertex buffer view)
		commandList->IASetIndexBuffer(&indexBufferView);

	}
	else
	{

		// set cube1's constant buffer
		commandList->SetGraphicsRootConstantBufferView(0, constantBufferUploadHeaps[frameIndex]->GetGPUVirtualAddress());

		CD3DX12_GPU_DESCRIPTOR_HANDLE Cube1tex(mainDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		commandList->SetGraphicsRootDescriptorTable(1, Cube1tex);
		Cube1tex.Offset(2, buffOffset);
		commandList->SetGraphicsRootDescriptorTable(2, Cube1tex);
		Cube1tex = CD3DX12_GPU_DESCRIPTOR_HANDLE(mainDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		Cube1tex.Offset(1, buffOffset);
		commandList->SetGraphicsRootDescriptorTable(3, Cube1tex);

		// draw second cube
		commandList->DrawIndexedInstanced(numCubeIndices, 1, 0, 0, 0);

		commandList->SetGraphicsRootConstantBufferView(0, constantBufferUploadHeaps[frameIndex]->GetGPUVirtualAddress() + ConstantBufferPerObjectAlignedSize);

		Cube1tex = (mainDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		commandList->SetGraphicsRootDescriptorTable(1, Cube1tex);
		Cube1tex.Offset(2, buffOffset);
		commandList->SetGraphicsRootDescriptorTable(2, Cube1tex);
		Cube1tex = CD3DX12_GPU_DESCRIPTOR_HANDLE(mainDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		Cube1tex.Offset(1, buffOffset);
		commandList->SetGraphicsRootDescriptorTable(3, Cube1tex);

		// draw second cube
		commandList->DrawIndexedInstanced(numCubeIndices, 1, 0, 0, 0);

		CD3DX12_GPU_DESCRIPTOR_HANDLE bill(mainDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		commandList->SetGraphicsRootDescriptorTable(1, bill);
		bill.Offset(2, buffOffset);
		commandList->SetGraphicsRootDescriptorTable(2, bill);
		bill = CD3DX12_GPU_DESCRIPTOR_HANDLE(mainDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		bill.Offset(3, buffOffset);
		commandList->SetGraphicsRootDescriptorTable(3, bill);

		commandList->IASetVertexBuffers(0, 1, &m_BillboardVertexView[0]); // set the vertex buffer (using the vertex buffer view)
		commandList->IASetIndexBuffer(&m_BillboardIndexView[0]);

		for (size_t i = m_BillboardCount; i--;)
		{
			commandList->SetGraphicsRootConstantBufferView(0, m_BillboardConstantBufferUploadHeaps[frameIndex]->GetGPUVirtualAddress() + (ConstantBufferPerObjectAlignedSize * i));
			commandList->DrawIndexedInstanced(numCubeIndices2, 1, 0, 0, 0);
		}

		commandList->SetPipelineState(pipelineStateObject);
		commandList->IASetVertexBuffers(0, 1, &vertexBufferView); // set the vertex buffer (using the vertex buffer view)
		commandList->IASetIndexBuffer(&indexBufferView);

	}

	if (m_RenderToTexture)
	{

		// here we start recording commands into the commandList (which all the commands will be stored in the commandAllocator)

		// transition the "frameIndex" render target from the present state to the render target state so the command list draws to it starting from here
		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(MSAA_RenderTarget, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

		// here we again get the handle to our current render target view so we can set it as the render target in the output merger stage of the pipeline
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), 4, rtvDescriptorSize);

		// get a handle to the depth/stencil buffer
		CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
		dsvHandle.Offset(1, dsvDescriptorSize);

		//dsvHandle.Offset(1, dsvDescriptorSize);

		// set the render target for the output merger stage (the output of the pipeline)
		commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

		// Clear the render target by using the ClearRenderTargetView command
		const float clearColor[] = { 0.0f, 0.4f, 0.4f, 1.0f };
		commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

		// clear the depth/stencil buffer
		commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		// set root signature
		commandList->SetGraphicsRootSignature(rootSignature); // set the root signature

		// set the descriptor heap
		ID3D12DescriptorHeap* descriptorHeaps[] = { mainDescriptorHeap };
		commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

		// set the descriptor table to the descriptor heap (parameter 1, as constant buffer root descriptor is parameter index 0)
		commandList->SetGraphicsRootDescriptorTable(1, mainDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

		commandList->RSSetViewports(1, &viewport); // set the viewports
		commandList->RSSetScissorRects(1, &scissorRect); // set the scissor rects
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // set the primitive topology
		commandList->IASetVertexBuffers(0, 1, &vertexBufferView); // set the vertex buffer (using the vertex buffer view)
		commandList->IASetIndexBuffer(&indexBufferView);

		// first cube

		//Book
		CD3DX12_GPU_DESCRIPTOR_HANDLE Cube1tex(mainDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		commandList->SetGraphicsRootDescriptorTable(1, Cube1tex);
		Cube1tex.Offset(2, buffOffset);
		commandList->SetGraphicsRootDescriptorTable(2, Cube1tex);
		Cube1tex = CD3DX12_GPU_DESCRIPTOR_HANDLE(mainDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		Cube1tex.Offset(3, buffOffset);
		commandList->SetGraphicsRootDescriptorTable(3, Cube1tex);

		// set cube1's constant buffer
		commandList->SetGraphicsRootConstantBufferView(0, constantBufferUploadHeaps[frameIndex]->GetGPUVirtualAddress());

		// draw first cube
		commandList->DrawIndexedInstanced(numCubeIndices, 1, 0, 0, 0);

		// second cube


		// set cube2's constant buffer. You can see we are adding the size of ConstantBufferPerObject to the constant buffer
		// resource heaps address. This is because cube1's constant buffer is stored at the beginning of the resource heap, while
		// cube2's constant buffer data is stored after (256 bits from the start of the heap).
		commandList->SetGraphicsRootConstantBufferView(0, constantBufferUploadHeaps[frameIndex]->GetGPUVirtualAddress() + ConstantBufferPerObjectAlignedSize);

		//Book
		CD3DX12_GPU_DESCRIPTOR_HANDLE Cube2tex(mainDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		Cube2tex.Offset(3, buffOffset);
		commandList->SetGraphicsRootDescriptorTable(1, Cube2tex);
		Cube2tex.Offset(0, buffOffset);
		commandList->SetGraphicsRootDescriptorTable(2, Cube2tex);
		Cube2tex = CD3DX12_GPU_DESCRIPTOR_HANDLE(mainDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		Cube2tex.Offset(3, buffOffset);
		commandList->SetGraphicsRootDescriptorTable(3, Cube2tex);

		// draw second cube
		commandList->DrawIndexedInstanced(numCubeIndices, 1, 0, 0, 0);

		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(MSAA_RenderTarget, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_RESOLVE_SOURCE));

		//renderTargets[frameIndex]
		commandList->ResolveSubresource(textureBuffer[4], 0, MSAA_RenderTarget, 0, DXGI_FORMAT_R8G8B8A8_UNORM);
	}

	if (m_ShadowMapping)
	{
		// copy our ConstantBuffer instance to the mapped constant buffer resource
		memcpy(cbvGPUAddress[frameIndex] + (ConstantBufferPerObjectAlignedSize * 4), &cbPerObject, sizeof(cbPerObject));

		//Need to ajust the constant buffer to change the light perspective and then reverse it i.e 	wvpMat = XMLoadFloat4x4(&CUBE WORLD POS) * lightView * lightProj; // create wvp matrix

		commandList->RSSetViewports(1, &m_SM.Viewport());
		commandList->RSSetScissorRects(1, &m_SM.Rect());

		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_SM.Resource(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE));

		commandList->ClearDepthStencilView(m_SM.Dsv(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

		commandList->OMSetRenderTargets(0, nullptr, false, &m_SM.Dsv());

		commandList->SetPipelineState(ShadowPipelineState);

		commandList->SetGraphicsRootConstantBufferView(0, constantBufferUploadHeaps[frameIndex]->GetGPUVirtualAddress() + (ConstantBufferPerObjectAlignedSize + ConstantBufferPerObjectAlignedSize + ConstantBufferPerObjectAlignedSize));

		commandList->DrawIndexedInstanced(numCubeIndices, 1, 0, 0, 0);

		commandList->SetGraphicsRootConstantBufferView(0, constantBufferUploadHeaps[frameIndex]->GetGPUVirtualAddress() + ConstantBufferPerObjectAlignedSize + ConstantBufferPerObjectAlignedSize + ConstantBufferPerObjectAlignedSize + ConstantBufferPerObjectAlignedSize);

		commandList->DrawIndexedInstanced(numCubeIndices, 1, 0, 0, 0);

		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_SM.Resource(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ));

	}

	// transition the "frameIndex" render target from the render target state to the present state. If the debug layer is enabled, you will receive a
	// warning if present is called on the render target when it's not in the present state
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));



	commandList->SetPipelineState(pipelineStateObject);

	hr = commandList->Close();
	if (FAILED(hr))
	{
		Running = false;
	}
	
}
void Render()
{
	HRESULT hr;

	UpdatePipeline(); // update the pipeline by sending commands to the commandqueue

	// create an array of command lists (only one command list here)
	ID3D12CommandList* ppCommandLists[] = { commandList };

	// execute the array of command lists
	commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// this command goes in at the end of our command queue. we will know when our command queue 
	// has finished because the fence value will be set to "fenceValue" from the GPU since the command
	// queue is being executed on the GPU
	hr = commandQueue->Signal(fence[frameIndex], fenceValue[frameIndex]);
	if (FAILED(hr))
	{
		Running = false;
	}

	// present the current backbuffer
	hr = swapChain->Present(0, 0);
	if (FAILED(hr))
	{
		hr = device->GetDeviceRemovedReason();
		Running = false;
	}
}

void Cleanup()
{
	// wait for the gpu to finish all frames
	for (int i = 0; i < frameBufferCount; ++i)
	{
		frameIndex = i;
		WaitForPreviousFrame();
	}

	// get swapchain out of full screen before exiting
	BOOL fs = false;
	if (swapChain->GetFullscreenState(&fs, NULL))
		swapChain->SetFullscreenState(false, NULL);

	SAFE_RELEASE(device);
	SAFE_RELEASE(swapChain);
	SAFE_RELEASE(commandQueue);
	SAFE_RELEASE(rtvDescriptorHeap);
	SAFE_RELEASE(commandList);

	for (int i = 0; i < frameBufferCount; ++i)
	{
		SAFE_RELEASE(renderTargets[i]);
		SAFE_RELEASE(commandAllocator[i]);
		SAFE_RELEASE(fence[i]);
	};

	for (int i = 0; i < 5; ++i)
	{
		SAFE_RELEASE(textureBuffer[i]);
	}

	SAFE_RELEASE(pipelineStateObject);
	SAFE_RELEASE(pipelineStateObject2);
	SAFE_RELEASE(ShadowPipelineState);
	SAFE_RELEASE(GeometryShaderPipeline);
	SAFE_RELEASE(rootSignature);
	SAFE_RELEASE(vertexBuffer);
	SAFE_RELEASE(indexBuffer);
	for (int i = 0; i < 4; ++i)
	{
		SAFE_RELEASE(textureBufferUploadHeap[i]);
	};

	for (size_t i = 0; i < m_BillboardVertex.size();i++)
	{
		SAFE_RELEASE(m_BillboardVertex[i]);
		SAFE_RELEASE(m_BillboardIndex[i]);
	}

	SAFE_RELEASE(depthStencilBuffer);
	SAFE_RELEASE(dsDescriptorHeap);
	SAFE_RELEASE(depthStencilBuffer2);

	for (int i = 0; i < frameBufferCount; ++i)
	{
		SAFE_RELEASE(constantBufferUploadHeaps[i]);
	};

}

void WaitForPreviousFrame()
{
	HRESULT hr;

	// swap the current rtv buffer index so we draw on the correct buffer
	frameIndex = swapChain->GetCurrentBackBufferIndex();

	// if the current fence value is still less than "fenceValue", then we know the GPU has not finished executing
	// the command queue since it has not reached the "commandQueue->Signal(fence, fenceValue)" command
	if (fence[frameIndex]->GetCompletedValue() < fenceValue[frameIndex])
	{
		// we have the fence create an event which is signaled once the fence's current value is "fenceValue"
		hr = fence[frameIndex]->SetEventOnCompletion(fenceValue[frameIndex], fenceEvent);
		if (FAILED(hr))
		{
			Running = false;
		}

		// We will wait until the fence has triggered the event that it's current value has reached "fenceValue". once it's value
		// has reached "fenceValue", we know the command queue has finished executing
		WaitForSingleObject(fenceEvent, INFINITE);
	}

	// increment fenceValue for next frame
	fenceValue[frameIndex]++;
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