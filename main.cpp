//https://github.com/mcbaze/vision_sensitivity_analyser - постоянная ссылка на версию с последними исправлениями
//
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#define LEFT_SIDE 200				//
#define TOP_SIDE 5					//
#define WND_WIDTH LEFT_SIDE+110		// для вёрстки
#define WND_HEIGHT 350				//
#define HORIZ_BOARDER 24			//
#define VERT_BOARDER 6				//

#define TIME_CONSTANT 25			//[мс],  1/TIME_CONSTANT = n кадров/секунду

#include "functions.h"

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);	
LRESULT CALLBACK picProc(HWND, UINT, WPARAM, LPARAM);	
LRESULT CALLBACK graphProc(HWND, UINT, WPARAM, LPARAM);

HWND hWnd, image_wnd, graph_wnd;
HWND re_ch, gr_ch, bl_ch, re_ed, gr_ed, bl_ed, nt_ed, speed_ed,  distance_ed, monitor_width_ed, repeat_ed;
HWND save_txt_btn, save_graph_btn, graph_panel, patient_name_ed;

CHAR re_l[3], gr_l[3], bl_l[3], nt_l[3], speed_l[4], distance_l[4], monitor_width_l[4], repeat_l[3];

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShow)
{
	WNDCLASSEXA wcex;
	wcex.cbSize = sizeof(WNDCLASSEXA);
	wcex.style          = CS_HREDRAW | CS_PARENTDC;
	wcex.lpfnWndProc    = WndProc;
	wcex.cbClsExtra     = 0;
	wcex.cbWndExtra     = 0;
	wcex.hInstance      = hInstance;
	wcex.hIcon          = NULL;
	wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW);
	wcex.lpszMenuName   = NULL;
	wcex.lpszClassName  = "mainWndclass";
	wcex.hIconSm        = NULL;
	
	WNDCLASSEXA pic_wnd;
	pic_wnd.cbSize = sizeof(WNDCLASSEXA);
	pic_wnd.style          = CS_PARENTDC;
	pic_wnd.lpfnWndProc    = picProc;
	pic_wnd.cbClsExtra     = 0;
	pic_wnd.cbWndExtra     = 0;
	pic_wnd.hInstance      = hInstance;
	pic_wnd.hIcon          = NULL;
	pic_wnd.hCursor        = NULL;
	pic_wnd.hbrBackground  = (HBRUSH)(COLOR_WINDOW);
	pic_wnd.lpszMenuName   = NULL;
	pic_wnd.lpszClassName  = "picWndclass";
	pic_wnd.hIconSm        = NULL;
	
	WNDCLASSEXA gr_wnd;
	gr_wnd.cbSize = sizeof(WNDCLASSEXA);
	gr_wnd.style          = CS_PARENTDC;
	gr_wnd.lpfnWndProc    = graphProc;
	gr_wnd.cbClsExtra     = 0;
	gr_wnd.cbWndExtra     = 0;
	gr_wnd.hInstance      = hInstance;
	gr_wnd.hIcon          = NULL;
	gr_wnd.hCursor        = NULL;
	gr_wnd.hbrBackground  = (HBRUSH)(COLOR_WINDOW);
	gr_wnd.lpszMenuName   = NULL;
	gr_wnd.lpszClassName  = "graphWndclass";
	gr_wnd.hIconSm        = NULL;

	if (!RegisterClassExA(&wcex) || !RegisterClassExA(&pic_wnd) || !RegisterClassExA(&gr_wnd))
	{
		MessageBoxA(NULL,"Ошибка при регистрации оконного класса.","Ошибка!",NULL);
		return 1;
	}
	hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	hWnd = CreateWindowA("mainWndclass","Тест_зрения",WS_SYSMENU,100,200,WND_WIDTH,WND_HEIGHT,NULL,NULL,hInstance,NULL);
	image_wnd = CreateWindowA("picWndclass","окно_теста",WS_POPUP,0,0,10,10,NULL,NULL,hInstance,NULL);
	graph_wnd = CreateWindowA("graphWndclass","окно_результатов",WS_SYSMENU,0,0,10,10,NULL,NULL,hInstance,NULL);
	if (!hWnd || !image_wnd || !graph_wnd)
	{
		MessageBoxA(NULL,"Ошибка при создании окон.", "Ошибка!", NULL);
		return 1;
	}
	ShowWindow(hWnd, SW_SHOW);
	xRes = GetDeviceCaps(GetDC(hWnd), HORZRES);
	yRes = GetDeviceCaps(GetDC(hWnd), VERTRES);
	line_Res = 10;
	pixel_count = line_Res*xRes;
	SetWindowPos(image_wnd, HWND_TOPMOST, 0, 0, xRes, yRes, NULL);
	SetWindowPos(graph_wnd, NULL, xRes/2-500, yRes/2-350, 1000, 700, NULL);
	srand((unsigned int)time(NULL));

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int) msg.wParam;
}

LRESULT CALLBACK WndProc(HWND wnd_proc_wnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case 1:
			GetWindowTextA(re_ed,re_l,4);
			GetWindowTextA(gr_ed,gr_l,4);
			GetWindowTextA(bl_ed,bl_l,4);
			GetWindowTextA(nt_ed,nt_l,4);
			GetWindowTextA(speed_ed,speed_l,5);
			GetWindowTextA(distance_ed,distance_l,5);
			GetWindowTextA(monitor_width_ed,monitor_width_l,5);
			GetWindowTextA(repeat_ed,repeat_l,4);
			k_r = IsDlgButtonChecked(hWnd,101);
			k_g = IsDlgButtonChecked(hWnd,102);
			k_b = IsDlgButtonChecked(hWnd,103);
			r_c = atoi(re_l)*k_r;
			g_c = atoi(gr_l)*k_g;
			b_c = atoi(bl_l)*k_b;
			n_t = atoi(nt_l);
			ampl = 0;
			speed = atof(speed_l)*0.001*TIME_CONSTANT;
			rep_number = 2*atoi(repeat_l);
			rep_count = rep_number;
			frequency = get_frequency(atof(distance_l),atof(monitor_width_l));
			sel_freq_treshold.resize(rep_number);
			direction = true;
			
			if((k_r)&&(k_g)&&(k_b)){
				max_c = max(r_c,max(g_c,b_c));
				min_c = min(r_c,min(g_c,b_c));}
			if((!k_r)&&(k_g)&&(k_b)){
				max_c = max(g_c,b_c);
				min_c = min(g_c,b_c);}
			if((k_r)&&(!k_g)&&(k_b)){
				max_c = max(r_c,b_c);
				min_c = min(r_c,b_c);}
			if((!k_r)&&(!k_g)&&(k_b)){
				max_c = k_b;
				min_c = k_b;}
			if((k_r)&&(k_g)&&(!k_b)){
				max_c = max(r_c,g_c);
				min_c = min(r_c,g_c);}
			if((!k_r)&&(k_g)&&(!k_b)){
				max_c = g_c;
				min_c = g_c;}
			if((k_r)&&(!k_g)&&(!k_b)){
				max_c = r_c;
				min_c = r_c;}
			if((!k_r)&&(!k_g)&&(!k_b)){
				max_c = 0;
				min_c = 0;}
			
			ShowWindow(image_wnd,SW_SHOW);
			UpdateWindow(image_wnd);
			ShowCursor(false);
			break;
		case 2:
			if(frequency_contrast_dep.size()>0)
			{
				for(byte i=0; i<frequency_contrast_dep.size(); i++){
					for(byte r=i+1; r<frequency_contrast_dep.size();r++){
						if(frequency_contrast_dep[i].first == frequency_contrast_dep[r].first)
						{
							frequency_contrast_dep.erase(frequency_contrast_dep.begin() + i);				
							i=0;
							r=1;
						}
					}
				}
				frequency_contrast_dep.shrink_to_fit();
				std::sort(frequency_contrast_dep.begin(),frequency_contrast_dep.end());
				min_tr = frequency_contrast_dep[0].second;
				max_tr = min_tr;
				for(byte i=1; i<frequency_contrast_dep.size(); i++){
					if (frequency_contrast_dep[i].second > max_tr)
					{
						max_tr = frequency_contrast_dep[i].second;
						continue;
					}
					if (frequency_contrast_dep[i].second < min_tr)
						min_tr = frequency_contrast_dep[i].second;
				}
				step_x = 974/(frequency_contrast_dep.size()+1);
				step_y = 560.0/(max_tr-min_tr);
				
				ShowWindow(graph_wnd,SW_SHOW);
				UpdateWindow(image_wnd);
			}
			else
			{
				MessageBoxA(NULL,"Нет данных для показа","Ошибка",MB_OK);
			}
		}
		break;
	case WM_CREATE:
		re_ch = CreateWindowA("button","R", WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX|BST_CHECKED, LEFT_SIDE, TOP_SIDE, 30, 18, wnd_proc_wnd,(HMENU)101,NULL,NULL);
		gr_ch = CreateWindowA("button","G", WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX|BST_CHECKED, LEFT_SIDE+35, TOP_SIDE, 30, 18, wnd_proc_wnd,(HMENU)102,NULL,NULL);
		bl_ch = CreateWindowA("button","B", WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX|BST_CHECKED, LEFT_SIDE+70, TOP_SIDE, 30, 18, wnd_proc_wnd,(HMENU)103,NULL,NULL);
		re_ed = CreateWindowA("edit","150", WS_CHILD|WS_VISIBLE|ES_CENTER|WS_BORDER, LEFT_SIDE, TOP_SIDE+20, 30, 20, wnd_proc_wnd,NULL,NULL,NULL);
		gr_ed = CreateWindowA("edit","150", WS_CHILD|WS_VISIBLE|ES_CENTER|WS_BORDER, LEFT_SIDE+35, TOP_SIDE+20, 30, 20, wnd_proc_wnd,NULL,NULL,NULL);
		bl_ed = CreateWindowA("edit","150", WS_CHILD|WS_VISIBLE|ES_CENTER|WS_BORDER, LEFT_SIDE+70, TOP_SIDE+20, 30, 20, wnd_proc_wnd,NULL,NULL,NULL);
		nt_ed = CreateWindowA("edit","1", WS_CHILD|WS_VISIBLE|ES_CENTER|WS_BORDER, LEFT_SIDE, TOP_SIDE+44, 30, 20, wnd_proc_wnd,NULL,NULL,NULL);
		speed_ed = CreateWindowA("edit","1.0", WS_CHILD|WS_VISIBLE|ES_CENTER|WS_BORDER, LEFT_SIDE, TOP_SIDE+68, 35, 20, wnd_proc_wnd,NULL,NULL,NULL);
		monitor_width_ed = CreateWindowA("edit","47.6", WS_CHILD|WS_VISIBLE|ES_CENTER|WS_BORDER,  LEFT_SIDE, TOP_SIDE+92, 35, 20, wnd_proc_wnd,NULL,NULL,NULL);
		distance_ed = CreateWindowA("edit","0.8", WS_CHILD|WS_VISIBLE|ES_CENTER|WS_BORDER,  LEFT_SIDE, TOP_SIDE+116, 35, 20, wnd_proc_wnd,NULL,NULL,NULL);
		repeat_ed = CreateWindowA("edit","1", WS_CHILD|WS_VISIBLE|ES_CENTER|WS_BORDER, LEFT_SIDE, TOP_SIDE+140, 30, 20, wnd_proc_wnd,NULL,NULL,NULL);
		SendMessageA(CreateWindowA("button", "Показать", WS_CHILD|WS_VISIBLE, 5, WND_HEIGHT-HORIZ_BOARDER-100,  WND_WIDTH-VERT_BOARDER-10, 45, wnd_proc_wnd, (HMENU)1,NULL, NULL), WM_SETFONT, (WPARAM)hFont, TRUE);
		SendMessageA(CreateWindowA("button", "Результат", WS_CHILD|WS_VISIBLE, 5, WND_HEIGHT-HORIZ_BOARDER-50,  WND_WIDTH-VERT_BOARDER-10, 45, wnd_proc_wnd, (HMENU)2,NULL, NULL), WM_SETFONT, (WPARAM)hFont, TRUE);
		SendMessageA(CreateWindowA("static","Каналы цвета",	WS_CHILD|WS_VISIBLE|SS_RIGHT, 5, TOP_SIDE+1, LEFT_SIDE-10, 20, wnd_proc_wnd,NULL,NULL,NULL), WM_SETFONT, (WPARAM)hFont, TRUE);
		SendMessageA(CreateWindowA("static","Интенсивность каналов", WS_CHILD|WS_VISIBLE|SS_RIGHT, 5, TOP_SIDE+22, LEFT_SIDE-10, 20, wnd_proc_wnd,NULL,NULL,NULL), WM_SETFONT, (WPARAM)hFont, TRUE);
		SendMessageA(CreateWindowA("static","Количество циклов", WS_CHILD|WS_VISIBLE|SS_RIGHT, 5, TOP_SIDE+46, LEFT_SIDE-10, 20, wnd_proc_wnd,NULL,NULL,NULL), WM_SETFONT, (WPARAM)hFont, TRUE);
		SendMessageA(CreateWindowA("static","Скорость изменения (ур./с)", WS_CHILD|WS_VISIBLE|SS_RIGHT, 5, TOP_SIDE+70, LEFT_SIDE-10, 20, wnd_proc_wnd,NULL,NULL,NULL), WM_SETFONT, (WPARAM)hFont, TRUE);
		SendMessageA(CreateWindowA("static","Ширина монитора (см)",	WS_CHILD|WS_VISIBLE|SS_RIGHT, 5, TOP_SIDE+94, LEFT_SIDE-10, 20, wnd_proc_wnd,NULL,NULL,NULL), WM_SETFONT, (WPARAM)hFont, TRUE);
		SendMessageA(CreateWindowA("static","Расст. от монитора (м)", WS_CHILD|WS_VISIBLE|SS_RIGHT, 5, TOP_SIDE+118, LEFT_SIDE-10, 20, wnd_proc_wnd,NULL,NULL,NULL), WM_SETFONT, (WPARAM)hFont, TRUE);
		SendMessageA(CreateWindowA("static","Кол-во повторов на частоте", WS_CHILD|WS_VISIBLE|SS_RIGHT, 5, TOP_SIDE+142, LEFT_SIDE-10, 20, wnd_proc_wnd,NULL,NULL,NULL), WM_SETFONT, (WPARAM)hFont, TRUE);
		SendMessageA(re_ch, BM_SETCHECK, (WPARAM)BST_CHECKED, TRUE);
		SendMessageA(gr_ch, BM_SETCHECK, (WPARAM)BST_CHECKED, TRUE);
		SendMessageA(bl_ch, BM_SETCHECK, (WPARAM)BST_CHECKED, TRUE);
		SendMessageA(re_ch, WM_SETFONT, (WPARAM)hFont, TRUE);
		SendMessageA(gr_ch, WM_SETFONT, (WPARAM)hFont, TRUE);
		SendMessageA(bl_ch, WM_SETFONT, (WPARAM)hFont, TRUE);
		SendMessageA(re_ed, WM_SETFONT, (WPARAM)hFont, TRUE);
		SendMessageA(gr_ed, WM_SETFONT, (WPARAM)hFont, TRUE);
		SendMessageA(bl_ed, WM_SETFONT, (WPARAM)hFont, TRUE);
		SendMessageA(nt_ed, WM_SETFONT, (WPARAM)hFont, TRUE);
		SendMessageA(speed_ed, WM_SETFONT, (WPARAM)hFont, TRUE);
		SendMessageA(monitor_width_ed, WM_SETFONT, (WPARAM)hFont, TRUE);
		SendMessageA(distance_ed, WM_SETFONT, (WPARAM)hFont, TRUE);
		SendMessageA(repeat_ed, WM_SETFONT, (WPARAM)hFont, TRUE);
		break;
	case WM_CLOSE:
		ShowCursor(true);
		exit(0);
		break;
    default:
        return DefWindowProc(wnd_proc_wnd, message, wParam, lParam);
        break;
    }
     return 0;
 }

LRESULT CALLBACK picProc(HWND pic_proc_wnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
	case WM_KEYDOWN:
		switch(LOWORD(wParam))
		{
		case VK_ESCAPE:
			ShowWindow(pic_proc_wnd, SW_HIDE);
			KillTimer(pic_proc_wnd,1);
			running = false;
			ShowCursor(true);
			break;
		case VK_SPACE:
			if (!running)
			{
				SetTimer(image_wnd,1,TIME_CONSTANT,NULL);
				running = true;
			}
			break;
		}
		break;
	case WM_KEYUP:
		switch(LOWORD(wParam))
		{
		case VK_SPACE:
			running = false;
			sel_freq_treshold[rep_count-1] = Get_treshold();
			if(direction)
				ampl+=2.0;
			else
				if(ampl<=2.0)
					ampl=0.0;
				else
					ampl-=2.0;
			direction = !direction;

			InvalidateRect(pic_proc_wnd, NULL, false);
			KillTimer(pic_proc_wnd,1);
			rep_count--;
			
			if(rep_count==0)
			{
				contrast = 0.0;
				for(byte i = 0; i<rep_number; i++)
					contrast += sel_freq_treshold[i];
				contrast /= rep_number;

				frequency_contrast_dep.push_back(std::make_pair(frequency,contrast));
				ShowWindow(pic_proc_wnd, SW_HIDE);
				KillTimer(pic_proc_wnd,1);
				ShowCursor(true);
			}
			break;
		}
		break;
	case WM_PAINT:
		Output_image(pic_proc_wnd);
		break;
	case WM_TIMER:
		change_ampl(direction);
		InvalidateRect(pic_proc_wnd, NULL, false);
		break;
	case WM_DESTROY:
		ShowCursor(true);
		exit(0);
		break;
    default:
        return DefWindowProc(pic_proc_wnd, message, wParam, lParam);
        break;
    }
    return 0;
}

LRESULT CALLBACK graphProc(HWND graph_proc_wnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	 switch (message)
    {
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case 201:
			GetWindowTextA(patient_name_ed,patient_name,74);
			lstrcatA(patient_name,".bmp");
			WriteBmp(patient_name, graph_proc_wnd);
			break;
		case 202:
			GetWindowTextA(patient_name_ed,patient_name,74);
			lstrcatA(patient_name,".txt");
			hFile = CreateFileA(patient_name, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			file_write_buffer[0] = 0;
			for(byte i=0; i<frequency_contrast_dep.size(); i++){
				_gcvt_s(text_buffer1,7,frequency_contrast_dep[i].first,5);
				lstrcatA(text_buffer1," ");
				_gcvt_s(text_buffer2,7,frequency_contrast_dep[i].second,5);
				lstrcatA(text_buffer1,text_buffer2);
				lstrcatA(file_write_buffer,text_buffer1);
				lstrcatA(file_write_buffer,"\r\n");
			}
			lstrcatA(file_write_buffer,"\0");
			WriteFile(hFile, file_write_buffer, strlen(file_write_buffer), &bytes_written,NULL);
			CloseHandle(hFile);
			break;
		}
		break;
	case WM_PAINT:
		Draw_graph(graph_proc_wnd);
		break;
	case WM_CREATE:
		CreateWindowA("static","", WS_CHILD|WS_VISIBLE|SS_BLACKFRAME, 20, 20, 954, 600, graph_proc_wnd,NULL,NULL,NULL);
		patient_name_ed = CreateWindowA("edit","Имя тестируемого", WS_CHILD|WS_VISIBLE|ES_LEFT|WS_BORDER, 20, 640, 500, 20, graph_proc_wnd,NULL,NULL,NULL);
		SendMessageA(patient_name_ed, WM_SETFONT, (WPARAM)hFont, TRUE);
		SendMessageA(CreateWindowA("button", "Сохранить график",WS_CHILD|WS_VISIBLE, 540, 630,  205, 40, graph_proc_wnd, (HMENU)201,NULL, NULL), WM_SETFONT, (WPARAM)hFont, TRUE);
		SendMessageA(CreateWindowA("button", "Сохранить в текстовый файл",WS_CHILD|WS_VISIBLE, 770, 630,  205, 40, graph_proc_wnd, (HMENU)202,NULL, NULL), WM_SETFONT, (WPARAM)hFont, TRUE);
		break;
	case WM_CLOSE:
		ShowWindow(graph_proc_wnd, SW_HIDE);
		break;
    default:
        return DefWindowProc(graph_proc_wnd, message, wParam, lParam);
        break;
    }
    return 0;
}
