#include <iostream>
#include <opencv2\opencv.hpp>
#include <Windows.h>
#include <cstdlib>


using namespace std;
using namespace cv;

String vPath = ""; /* Bilgisayardan se�ti�imiz videonun yolunu tutan de�i�ken */
String templPath = ""; /* Bilgisayardan se�ti�imiz �ablon foto�raf�n yolunu tutan de�i�ken */
Mat templ;  Mat result; Mat frame;
Point point1, point2 , point3; 
/* point1: Video ba�lad���nda �ablon foto�raf� belirlemek i�in t�klad���m�z ilk noktan�n koordinatlar�n� tutar */
/* point2: Video ba�lad���nda �ablon foto�raf� belirlemek i�in sol kli�i kald�rd���m�z ilk noktan�n koordinatlar�n� tutar */
/* point3: Men� penceresinde t�klad���m�z koordinatlar� tutar */
int drag = 0;  /* Video oynarken sol kli�e t�klad���m�zda 1, kald�rd���m�zda 0 olur */
Rect rect; /* Videoda olu�turdu�umuz kutunun �eklini belirlen */
Mat roiImg; /* roiImg - olu�turulan kutudaki resmin bir par�as� */
const char* image_window = "Kaynak Video";
const char* result_window = "Sonu� Penceresi";
int match_method;
int max_Trackbar = 5;
bool playVideo = true; /* True is video devam eder, false ise video durur */

void MatchingMethod(int, void*);  //�ablon g�r�nt�yle frame'de e�le�en (veya benzer) alan� bulmak i�in kullan�lan metod
int Run(); //Men�de �al��t�r butonun bas�ld���nda i�lemi ba�latan buton

string buttonText("Video Bul..");
string buttonText2("Aranacak Sablon Fotograf..");
string buttonText3("Calistir..");
string buttonText4("Cikis..");
string winName = "Men�";

Mat img(200, 250, CV_8UC3, Scalar(255, 255, 255)); // Men� arkaplan

//Konsol ekran�n� saklayan fonksiyon
void HideConsole()
{
	::ShowWindow(::GetConsoleWindow(), SW_HIDE);
}


//Men� penceresinde ki i�lemleri yapan fonksiyon
void callBackFunc(int event, int x, int y, int flags, void* userdata)
{
	HWND hWnd = NULL;
	char mname[256];
	::GetConsoleTitle(mname, 256);                                    // <-- Konsol ba�l���n� getir
	hWnd = FindWindow("ConsoleWindowClass", mname);  // O isimdeki konsol penceresini bul
	OPENFILENAME ofn;
	::memset(&ofn, 0, sizeof(ofn));
	char f1[MAX_PATH];
	f1[0] = 0;

	//Men�de sol klik t�kland���nda
	if (event == EVENT_LBUTTONDOWN)
	{
		point3 = Point(x, y);

		//Men�deki her buton 50pixel y�ksekli�inde oldu�u i�in her 50 pixel i�inde, o butona g�re i�lem yapar
		//Video Bul butonuna bas�ld���nda
		if (point3.x < img.rows && point3.y < 50)
		{
			ofn.lStructSize = sizeof(ofn);
			ofn.lpstrTitle = "Bir Video Dosyas� Se�in";
			ofn.lpstrFilter = "Text Files\0*.txt\0All Files\0*.*\0\0";
			ofn.nFilterIndex = 2;
			ofn.lpstrFile = f1;
			ofn.nMaxFile = MAX_PATH;
			ofn.hwndOwner = hWnd;       // <-- Yeni Sat�r
			ofn.Flags = OFN_FILEMUSTEXIST;

			if (::GetOpenFileName(&ofn) != FALSE)
			{
				vPath = ofn.lpstrFile;  // Se�ilen videonun bilgisayardaki yolunu "vPath" de�i�kenine atar.
				destroyWindow("error");
			}
		
		}

		//�ablon Se� butonuna bas�ld���nda
		if (point3.x < img.rows && point3.y >= 50 && point3.y <100)
		{
			ofn.lStructSize = sizeof(ofn);
			ofn.lpstrTitle = "Bir �ablon Foto�raf� Se�in";
			ofn.lpstrFilter = "Text Files\0*.txt\0All Files\0*.*\0\0";
			ofn.nFilterIndex = 2;
			ofn.lpstrFile = f1;
			ofn.nMaxFile = MAX_PATH;
			ofn.hwndOwner = hWnd;       // <-- Yeni sat�r
			ofn.Flags = OFN_FILEMUSTEXIST;

			if (::GetOpenFileName(&ofn) != FALSE)
			{
			    templPath = ofn.lpstrFile;  // Se�ilen �ablon foto�raf�n bilgisayardaki yolunu "templPath" de�i�kenine atar.
			}
			
		}

		//�al��t�r butonuna bas�ld���nda
		if (point3.x < img.rows && point3.y >= 100 && point3.y <150)
		{
			//Video se�ilmediyse uyar� penceresi verir
			if (vPath == "")
			{
				Mat err_img(50, 200, CV_8UC3, Scalar(0, 255, 255));
				putText(err_img, "Lutfen video secin." , Point(5, 30), FONT_HERSHEY_PLAIN, 1, Scalar(0, 0, 0));
				imshow("error", err_img);

			}
			else
			Run();

		}

		//��k�� butonuna bas�ld���nda
		if (point3.x < img.rows && point3.y >= 150 && point3.y <200)
		{
			exit(0);
		}

	}
	
}


//Video oynarken mouse ile �zerinde yap�lan i�lemler
void mouseHandler(int event, int x, int y, int flags, void *param)
{
	//Mouse sa� klik t�kland���nda video durdurulur
	if (event == CV_EVENT_RBUTTONDOWN && !drag)
	{
		playVideo = !playVideo;
	}

	//Mouse sol klik t�kland���nda �izdi�imiz dikd�rtgenin point1 noktas� olu�ur
	if (event == CV_EVENT_LBUTTONDOWN && !drag)
	{

		point1 = Point(x, y);
		drag = 1;
		cout << "point1.x..:" << point1.x << endl << "point1.y..: " << point1.y << endl;
	}

	//Mouse sol klik bas�l�yken oynatt���m�zda o koordinatta kutuyu olu�turur
	if (event == CV_EVENT_MOUSEMOVE && drag)
	{

		Mat img1 = frame.clone();
		point2 = Point(x, y);
		rectangle(img1, point1, point2, CV_RGB(255, 0, 0), 3, 8, 0);
		imshow(image_window, img1);
	}

	//Mouse sol klik kald�r�ld���nda �ablon olu�turulur
	if (event == CV_EVENT_LBUTTONUP && drag)
	{
		point2 = Point(x, y);
		//�ablon �ok k���k oldu�unda video durur
		if (x-point1.x<10 || y-point1.y<10)
			playVideo = false;

		if(y < point1.y && x > point1.x) //Videoda kutuyu olu�tururken sol alttan sa� �ste do�ru olu�turdu�umuzda �al���r
			rect = Rect(point1.x, y, x - point1.x, point1.y - y);
		else if (y > point1.y && x < point1.x) //Videoda kutuyu olu�tururken sa� �stten sol a�a�� do�ru olu�turdu�umuzda �al���r
			rect = Rect(x, point1.y, point1.x-x, y-point1.y);
		else if (y < point1.y && x < point1.x) //Videoda kutuyu olu�tururken sa� alttan sol �ste do�ru olu�turdu�umuzda �al���r
			rect = Rect(x, y, point1.x-x, point1.y - y);
		else  //Videoda kutuyu olu�tururken sol �stten sa� a�a�� do�ru olu�turdu�umuzda �al���r
			rect = Rect(point1.x, point1.y, (x - point1.x) , (y - point1.y) );

		drag = 0;

		roiImg = frame(rect);
		roiImg.copyTo(templ); // Se�ti�imiz kutuyu "templ" ad�ndaki �ablon foto�raf� tutan de�i�kene atar ve videoda onu aramaya ba�lar

	}

	//Mouse orta klik t�kland���nda video kapan�r ve men�ye d�ner
	if (event == CV_EVENT_MBUTTONDOWN)
	{
		
		destroyWindow(image_window);
		destroyWindow(result_window);
		waitKey();

	}

}

//Men� olu�turuluyor
void Menu()
{

	//Men�deki dikd�rtgenler �izer
	rectangle(img, Point(0, 0), Point(250, 50), CV_RGB(169, 169, 169), -1, 8, 0);
	rectangle(img, Point(0, 50), Point(250, 100), CV_RGB(30, 144, 255), -1, 8, 0);
	rectangle(img, Point(0, 100), Point(250, 150), CV_RGB(0, 255, 0), -1, 8, 0);
	rectangle(img, Point(0, 150), Point(250, 200), CV_RGB(220, 20, 60), -1, 8, 0);
	
	//Men�deki yaz�lar ekler
	putText(img, buttonText, Point(80, 30), FONT_HERSHEY_PLAIN, 1, Scalar(0, 0, 0));
	putText(img, buttonText2, Point(12, 80), FONT_HERSHEY_PLAIN, 1, Scalar(0, 0, 0));
	putText(img, buttonText3, Point(90, 130), FONT_HERSHEY_PLAIN, 1, Scalar(0, 0, 0));
	putText(img, buttonText4, Point(100, 180), FONT_HERSHEY_PLAIN, 1, Scalar(0, 0, 0));



	namedWindow(winName); // Pencere ismini verir
	setMouseCallback(winName, callBackFunc); // Men�de yapt���m�z i�lemlerin fonksiyonu

	imshow(winName, img); //Men�y� ekrana getirir
	waitKey();
}

int Run()
{
	
	string fileName = vPath; //videonun yolu fileName de�i�keninde tutulur
	VideoCapture capture(fileName); // capture de�i�keni videoyu al�r

	string imgDir = templPath; // �ablon foto�raf�n yolu "imgDir" de�i�keninde tutulut
	//�ablon Foto�raf se�ilmemi�se bo� beyaz bir sayfa varsay�lan olarak ayarlan�r
	if (imgDir == "")
	{
		Mat img_temp(50, 50, CV_8UC3, Scalar(255, 255, 255));
		templ = img_temp;
	}

	else
		templ = imread(imgDir, IMREAD_COLOR); // �ablon resmi templ de�i�kenine atar

	//Video okunmad���nda hata yollar
	if (!capture.isOpened()) 
		throw "Video okunurken hata";

	//Pencere ismi verilir
	namedWindow(image_window, WINDOW_AUTOSIZE);

	//Framelerin pe�pe�e geldi�i sonsuz d�ng� olu�turulur
	for (; ; )
	{
		//Video durdurulmad�ysa framler pe�pe�e gelmeye devam eder
		if (playVideo)
			capture >> frame;

		//video bittiyse sonsuz d�ng�den ��k�l�r
		if (frame.empty())
			break;

		//Videonun �st�nde bar i�indeki metodlar
		const char* trackbar_label = "Metod   ";
		createTrackbar(trackbar_label, image_window, &match_method, max_Trackbar, MatchingMethod);
		//Video i�inde �ablon se�mek istersek fonksiyon �al���r
		cvSetMouseCallback(image_window, mouseHandler, NULL);
		MatchingMethod(0, 0); // E�le�tirme metodu
		waitKey(10); // frame bekleme s�resi

	}
	//Video bitti�inde pencereler kapat�l�r ve men�ye d�ner
	destroyWindow(image_window);
	destroyWindow(result_window);
	waitKey();

}


//�ablon g�r�nt�yle frame'de e�le�en (veya benzer) alan� bulmak i�in kullan�lan metod
void MatchingMethod(int, void*)
{
	Mat img_display;
	frame.copyTo(img_display);
	int result_cols = frame.cols - templ.cols + 1;
	int result_rows = frame.rows - templ.rows + 1;

	matchTemplate(frame, templ, result, match_method); // Se�ilen nesneyi arayan fonksiyon

	normalize(result, result, 0, 1, NORM_MINMAX, -1, Mat());
	double minVal; double maxVal; Point minLoc; Point maxLoc;
	Point matchLoc;

	//Minimum ve maksimum eleman de�erlerini ve konumlar�n� bulur
	minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, Mat());
	if (match_method == TM_SQDIFF || match_method == TM_SQDIFF_NORMED)
	{
		matchLoc = minLoc;
	}
	else
	{
		matchLoc = maxLoc;
	}
	//�ablonn e�le�ti�i b�lge kutu i�ine al�n�r
	rectangle(img_display, matchLoc, Point(matchLoc.x + templ.cols, matchLoc.y + templ.rows), Scalar::all(0), 2, 8, 0);
	//Sonu� penceresinde �ablonun e�le�ti�i b�lge kutu i�ine al�n�r
	rectangle(result, matchLoc, Point(matchLoc.x + templ.cols, matchLoc.y + templ.rows), Scalar::all(0), 2, 8, 0);
	imshow(result_window, result); // Video penceresi ekrana getirilir
	imshow(image_window, img_display); // Sonu� penceresi ekrana getirilir
	return;
}

int main(int argc, char** argv)
{
	HideConsole();

	Menu();

	return 0;
}

