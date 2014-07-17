#include <Windows.h>
#include <math.h>
#include <vector>
#include <algorithm>
#include <time.h>

HDC hdc, memDC;
HBITMAP memBM;
HPEN hPen;
HFONT hFont;
HANDLE hFile;
HANDLE hOld1, hOld2;
PAINTSTRUCT ps;

unsigned long pixel_count;
unsigned int step_x;
DWORD bytes_written;
byte  r_c, g_c, b_c, ret_c, rep_number, rep_count, k_r, k_g, k_b, max_c, min_c;
int xRes, yRes, line_Res, x, y, n_t;
CHAR text_buffer1[14], text_buffer2[7], file_write_buffer[1401];
CHAR patient_name[80];
bool direction, running;
double speed, sum_c, delta, ampl, frequency, contrast, min_tr, max_tr, step_y;

std::vector<double> sel_freq_treshold(1);
std::vector<std::pair<double,double>> frequency_contrast_dep;		// зависимость порогового зн. от частоты 

byte color_calc(byte c1, double diff)
{
	sum_c = (double)c1 + diff;
	ret_c = (byte)sum_c;
	if ((sum_c - (double)ret_c + 0.1*(rand() % 10)) > 1.0)
		ret_c++;
	return ret_c;
}	 

void Output_image(HWND window)
{
	hdc = BeginPaint(window, &ps);
	memDC = CreateCompatibleDC(hdc);
    memBM = CreateCompatibleBitmap(hdc, xRes, line_Res);
    hOld1 = SelectObject(memDC, memBM);

	for(y = 0; y < line_Res; y++)
	{
		for(x = 0; x < xRes; x++ )
		{
      		delta = ampl * sin((6.28318 * n_t * x) / xRes);
			SetPixelV(memDC, x, y, RGB(color_calc(r_c, delta) * k_r, color_calc(g_c, delta) * k_g, color_calc(b_c, delta) * k_b));
		}
	}

	for (y = 0; y <= yRes; y += line_Res)
	{
		BitBlt(hdc, 0, y, xRes, line_Res, memDC, 0, 0, SRCCOPY);
	}

	SelectObject(memDC, hOld1);
    DeleteObject(memBM);
    DeleteDC(memDC);
	EndPaint(window, &ps);
}

double Get_treshold()
{
	double middle_brightness = 0.299*(r_c*k_r) + 0.587*(g_c*k_g) + 0.114*(b_c*k_b);
	if(ampl<0.01)
		return 0.0;
	return middle_brightness/ampl;
}

void Draw_graph(HWND graph_window)
{
	hdc = BeginPaint(graph_window, &ps);
	SetBkMode(hdc, TRANSPARENT);
	hPen = CreatePen(PS_SOLID, 2, RGB(0,0,0));
	hOld1 = SelectObject(hdc, hPen);
	hOld2 = SelectObject(hdc, hFont);
	
	MoveToEx(hdc, step_x, 600, NULL);
	LineTo(hdc, step_x, 618);
	_gcvt_s(text_buffer2,7,frequency_contrast_dep[0].first,5);
	TextOutA(hdc, step_x+10, 600, text_buffer2, 6);
	MoveToEx(hdc,step_x,590-(int)(step_y*(frequency_contrast_dep[0].second - min_tr)),NULL);
	for(byte i=1; i<frequency_contrast_dep.size(); i++){
		LineTo(hdc,step_x*(i+1),590-(int)(step_y*(frequency_contrast_dep[i].second - min_tr)));
		MoveToEx(hdc,step_x*(i+1),600,NULL);
		LineTo(hdc,step_x*(i+1),618);
		_gcvt_s(text_buffer2,7,frequency_contrast_dep[i].first,5);
		TextOut(hdc, step_x*(i+1)+10, 600, text_buffer2, 6);
		MoveToEx(hdc,step_x*(i+1),590-(int)(step_y*(frequency_contrast_dep[i].second - min_tr)),NULL);
	}

	SelectObject(hdc, hOld1);
	DeleteObject(hPen);
	EndPaint(graph_window, &ps);
};

void change_ampl(bool dir)
{
	if(dir&&(ampl<(255-max_c-2-speed))&&(ampl<(min_c-2-speed)))
		ampl += speed;
	else if((!dir)&&(ampl>0))
			ampl -= speed;
}

double get_frequency(double dist, double mon_width)
{
	return (0.5*n_t)/atan(0.005*mon_width/dist);
}

void WriteBmp(LPCSTR name, HWND window)
{
    BITMAPFILEHEADER bmfh = {0};
    BITMAPINFOHEADER bmih = {0};
    HDC hdc1;
    HDC hdc2;
    HBITMAP aBmp;
    BITMAPINFO bi;
    HGDIOBJ OldObj;
    void *dibvalues;
    DWORD bytes_write;
 
    hdc1 = GetWindowDC(window);
    hdc2 = CreateCompatibleDC(hdc1);

    bmih.biSize = sizeof(BITMAPINFOHEADER);
    bmih.biWidth = 954;
    bmih.biHeight = 600;
    bmih.biPlanes = 1;
    bmih.biBitCount = 24;
    bmih.biCompression = BI_RGB;
    bmih.biSizeImage = ((((bmih.biWidth * bmih.biBitCount) + 31) & ~31) >> 3) * bmih.biHeight;
    bi.bmiHeader = bmih;
 
    aBmp = CreateDIBSection(hdc1, &bi, DIB_RGB_COLORS, (void**)&dibvalues, NULL, NULL);
    OldObj = SelectObject(hdc2, aBmp);
    BitBlt(hdc2, 0, 0, 954, 600, hdc1, 23, 42, SRCCOPY);
    bmfh.bfOffBits = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER);
    bmfh.bfSize = (3*bmih.biHeight*bmih.biWidth)+sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER);
    bmfh.bfType = 0x4d42;
 
    hFile = CreateFile(name, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    bytes_write = sizeof(BITMAPFILEHEADER);
    WriteFile(hFile, &bmfh, bytes_write, &bytes_written, NULL);
    bytes_write = sizeof(BITMAPINFOHEADER); 
    WriteFile(hFile, &bmih, bytes_write, &bytes_written, NULL);
    bytes_write = bmih.biSizeImage;
    WriteFile(hFile, (void*)dibvalues, bytes_write, &bytes_written, NULL);
 
    CloseHandle(hFile);
    DeleteObject(SelectObject(hdc2,OldObj));
    DeleteDC(hdc2); 
    ReleaseDC(window, hdc1);
}
