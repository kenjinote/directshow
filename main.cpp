#pragma comment(linker,"/opt:nowin98")
#pragma comment(lib,"strmBasd.lib")
#pragma comment(lib,"Quartz.lib")
#include <windows.h>
#include <dShow.h>

#define IDU_BUTTON1 100
#define IDU_BUTTON2 101

CHAR szClassName[]="window";
BOOL b=FALSE;
IGraphBuilder *g_pGrph=NULL;
IMediaControl *g_pMdaCtrl=NULL;
IVideoWindow *g_pVdoWin=NULL;
HWND hButton1;

void ReleaseAll(void)
{
	if(g_pVdoWin){
		g_pVdoWin->Release();
		g_pVdoWin=NULL;
	}
	if(g_pMdaCtrl){
		g_pMdaCtrl->Release();
		g_pMdaCtrl=NULL;
	}
	if(g_pGrph){
		g_pGrph->Release();
		g_pGrph=NULL;
	}
}

void Play(HWND hWnd,LPCWSTR wszFileName)
{
	HRESULT hResult;
	if(b)return;
	hResult=CoCreateInstance(CLSID_FilterGraph,NULL,CLSCTX_INPROC_SERVER,IID_IGraphBuilder,(LPVOID *)&g_pGrph);
	if(FAILED(hResult)){
		MessageBox(hWnd,"インスタンス取得失敗","Error",MB_OK|MB_ICONSTOP);
		return;
	}
	hResult=g_pGrph->QueryInterface(IID_IMediaControl,(LPVOID *)&g_pMdaCtrl);
	if(FAILED(hResult)){
		ReleaseAll();
		return;
	}
	hResult=g_pGrph->QueryInterface(IID_IVideoWindow,(LPVOID *)&g_pVdoWin);
	if(FAILED(hResult)){
		ReleaseAll();
		return;
	}
	hResult=g_pGrph->RenderFile(wszFileName,NULL);
	if(FAILED(hResult)){
		ReleaseAll();
		return;
	}
	g_pVdoWin->put_Owner((OAHWND)hWnd);
	g_pVdoWin->put_WindowStyle(WS_CHILD|WS_CLIPSIBLINGS);
	g_pVdoWin->SetWindowPosition(0,90,640,480);
	hResult=g_pMdaCtrl->Run();
	if(FAILED(hResult)){
		ReleaseAll();
		return;
	}
	b=TRUE;SetWindowText(hButton1,"停止");
}

void Stop(void)
{
	HRESULT hResult;
	if(!b)return;
	hResult=g_pMdaCtrl->Stop();
	g_pVdoWin->put_Visible(OAFALSE);
	g_pVdoWin->put_Owner(NULL);
	ReleaseAll();
	b=FALSE;SetWindowText(hButton1,"再生");
}

LRESULT CALLBACK WndProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	static HWND hEdit1;
	char szFileName[MAX_PATH]="";
	WCHAR wszFileName[260];
	OPENFILENAME ofn;
	switch(msg){
	case WM_CREATE:
		CreateWindow("STATIC","ファイル名:",WS_CHILD|WS_VISIBLE,10,10,120,28,hWnd,NULL,((LPCREATESTRUCT)lParam)->hInstance,NULL);
		hEdit1=CreateWindowEx(WS_EX_CLIENTEDGE,"EDIT","",WS_CHILD|WS_VISIBLE,140,10,400,28,hWnd,NULL,((LPCREATESTRUCT)lParam)->hInstance,NULL);
		CreateWindow("BUTTON","...",WS_CHILD|WS_VISIBLE,542,10,28,28,hWnd,(HMENU)IDU_BUTTON1,((LPCREATESTRUCT)lParam)->hInstance,NULL);
		hButton1=CreateWindow("BUTTON","再生",WS_CHILD|WS_VISIBLE,10,50,150,28,hWnd,(HMENU)IDU_BUTTON2,((LPCREATESTRUCT)lParam)->hInstance,NULL);
		break;
	case WM_GETMINMAXINFO:
		{// 窓の最小サイズを設定
			MINMAXINFO* lpMMI=(MINMAXINFO*)lParam;
			lpMMI->ptMinTrackSize.x=560;
			lpMMI->ptMinTrackSize.y=140;
		}
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam)) 
		{
		case IDU_BUTTON1:
			ZeroMemory(&ofn,sizeof(OPENFILENAME));
			ofn.lStructSize=sizeof(OPENFILENAME);
			ofn.hwndOwner=hWnd;
			ofn.lpstrFilter="DirectShow Supported Files\0*.wma;*.wmv;*.asf;*.mpeg;*.mpg;*.avi;*.mov;*.wav;*.au;*.mp3;*.mid;*.midi\0All Files(*.*)\0*.*\0\0";
			ofn.nFilterIndex=1;
			ofn.lpstrFile=szFileName;
			ofn.nMaxFile=MAX_PATH;
			ofn.Flags=OFN_FILEMUSTEXIST|OFN_HIDEREADONLY;
			ofn.lpstrDefExt="";
			ofn.nMaxFileTitle=64;
			ofn.lpstrFileTitle=NULL;
			ofn.lpstrTitle=NULL;
			if(GetOpenFileName(&ofn))SetWindowText(hEdit1,szFileName);
			break;
		case IDU_BUTTON2:
			if(!b&&GetWindowTextLength(hEdit1)){
				GetWindowText(hEdit1,szFileName,MAX_PATH);
				MultiByteToWideChar(CP_ACP,0,szFileName,-1,wszFileName,260);
				Play(hWnd,wszFileName);
			}else{
				Stop();
			}
			break;
		default:
			return DefWindowProc(hWnd,msg,wParam,lParam);
		}
		break;
		case WM_DESTROY:
			Stop();
			PostQuitMessage(0);
			break;
		default:
			return(DefWindowProc(hWnd,msg,wParam,lParam));
	}
	return(0L);
}

int WINAPI WinMain(HINSTANCE hinst,HINSTANCE hPreInst,
				   LPSTR pCmdLine,int nCmdShow)
{
	HWND hWnd;
	MSG msg;
	WNDCLASS wndclass;
	HRESULT hResult;
	hResult=CoInitialize(NULL);
	if(FAILED(hResult)){ 
		MessageBox(NULL,"COMライブラリ初期化失敗","Error",MB_OK|MB_ICONSTOP);
		return FALSE;
	}
	if(!hPreInst){
		wndclass.style=CS_HREDRAW|CS_VREDRAW;
		wndclass.lpfnWndProc=WndProc;
		wndclass.cbClsExtra=0;
		wndclass.cbWndExtra=0;
		wndclass.hInstance=hinst;
		wndclass.hIcon=NULL;
		wndclass.hCursor=LoadCursor(NULL,IDC_ARROW);
		wndclass.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);
		wndclass.lpszMenuName=NULL;
		wndclass.lpszClassName=szClassName;
		if(!RegisterClass(&wndclass))
			return FALSE;
	}
	hWnd=CreateWindow(szClassName,
		"DirectShowを使ってオーディオ/ビデオ ファイルを再生する",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		NULL,
		NULL,
		hinst,
		NULL);
	ShowWindow(hWnd,nCmdShow);
	UpdateWindow(hWnd);
	while(GetMessage(&msg,NULL,0,0)){
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	CoUninitialize();
	return(msg.wParam);
}