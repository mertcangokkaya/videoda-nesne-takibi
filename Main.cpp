#include <iostream>
#include <opencv2\opencv.hpp>
#include <Windows.h>
#include <cstdlib>


using namespace std;
using namespace cv;

String vPath = ""; /* Bilgisayardan seçtiðimiz videonun yolunu tutan deðiþken */
String templPath = ""; /* Bilgisayardan seçtiðimiz þablon fotoðrafýn yolunu tutan deðiþken */
Mat templ;  Mat result; Mat frame;
Point point1, point2 , point3; 
/* point1: Video baþladýðýnda þablon fotoðrafý belirlemek için týkladýðýmýz ilk noktanýn koordinatlarýný tutar */
/* point2: Video baþladýðýnda þablon fotoðrafý belirlemek için sol kliði kaldýrdýðýmýz ilk noktanýn koordinatlarýný tutar */
/* point3: Menü penceresinde týkladýðýmýz koordinatlarý tutar */
int drag = 0;  /* Video oynarken sol kliðe týkladýðýmýzda 1, kaldýrdýðýmýzda 0 olur */
Rect rect; /* Videoda oluþturduðumuz kutunun þeklini belirlen */
Mat roiImg; /* roiImg - oluþturulan kutudaki resmin bir parçasý */
const char* image_window = "Kaynak Video";
const char* result_window = "Sonuç Penceresi";
int match_method;
int max_Trackbar = 5;
bool playVideo = true; /* True is video devam eder, false ise video durur */

void MatchingMethod(int, void*);  //Þablon görüntüyle frame'de eþleþen (veya benzer) alaný bulmak için kullanýlan metod
int Run(); //Menüde çalýþtýr butonun basýldýðýnda iþlemi baþlatan buton

string buttonText("Video Bul..");
string buttonText2("Aranacak Sablon Fotograf..");
string buttonText3("Calistir..");
string buttonText4("Cikis..");
string winName = "Menü";

Mat img(200, 250, CV_8UC3, Scalar(255, 255, 255)); // Menü arkaplan

//Konsol ekranýný saklayan fonksiyon
void HideConsole()
{
	::ShowWindow(::GetConsoleWindow(), SW_HIDE);
}


//Menü penceresinde ki iþlemleri yapan fonksiyon
void callBackFunc(int event, int x, int y, int flags, void* userdata)
{
	HWND hWnd = NULL;
	char mname[256];
	::GetConsoleTitle(mname, 256);                                    // <-- Konsol baþlýðýný getir
	hWnd = FindWindow("ConsoleWindowClass", mname);  // O isimdeki konsol penceresini bul
	OPENFILENAME ofn;
	::memset(&ofn, 0, sizeof(ofn));
	char f1[MAX_PATH];
	f1[0] = 0;

	//Menüde sol klik týklandýðýnda
	if (event == EVENT_LBUTTONDOWN)
	{
		point3 = Point(x, y);

		//Menüdeki her buton 50pixel yüksekliðinde olduðu için her 50 pixel içinde, o butona göre iþlem yapar
		//Video Bul butonuna basýldýðýnda
		if (point3.x < img.rows && point3.y < 50)
		{
			ofn.lStructSize = sizeof(ofn);
			ofn.lpstrTitle = "Bir Video Dosyasý Seçin";
			ofn.lpstrFilter = "Text Files\0*.txt\0All Files\0*.*\0\0";
			ofn.nFilterIndex = 2;
			ofn.lpstrFile = f1;
			ofn.nMaxFile = MAX_PATH;
			ofn.hwndOwner = hWnd;       // <-- Yeni Satýr
			ofn.Flags = OFN_FILEMUSTEXIST;

			if (::GetOpenFileName(&ofn) != FALSE)
			{
				vPath = ofn.lpstrFile;  // Seçilen videonun bilgisayardaki yolunu "vPath" deðiþkenine atar.
				destroyWindow("error");
			}
		
		}

		//Þablon Seç butonuna basýldýðýnda
		if (point3.x < img.rows && point3.y >= 50 && point3.y <100)
		{
			ofn.lStructSize = sizeof(ofn);
			ofn.lpstrTitle = "Bir Þablon Fotoðrafý Seçin";
			ofn.lpstrFilter = "Text Files\0*.txt\0All Files\0*.*\0\0";
			ofn.nFilterIndex = 2;
			ofn.lpstrFile = f1;
			ofn.nMaxFile = MAX_PATH;
			ofn.hwndOwner = hWnd;       // <-- Yeni satýr
			ofn.Flags = OFN_FILEMUSTEXIST;

			if (::GetOpenFileName(&ofn) != FALSE)
			{
			    templPath = ofn.lpstrFile;  // Seçilen þablon fotoðrafýn bilgisayardaki yolunu "templPath" deðiþkenine atar.
			}
			
		}

		//Çalýþtýr butonuna basýldýðýnda
		if (point3.x < img.rows && point3.y >= 100 && point3.y <150)
		{
			//Video seçilmediyse uyarý penceresi verir
			if (vPath == "")
			{
				Mat err_img(50, 200, CV_8UC3, Scalar(0, 255, 255));
				putText(err_img, "Lutfen video secin." , Point(5, 30), FONT_HERSHEY_PLAIN, 1, Scalar(0, 0, 0));
				imshow("error", err_img);

			}
			else
			Run();

		}

		//Çýkýþ butonuna basýldýðýnda
		if (point3.x < img.rows && point3.y >= 150 && point3.y <200)
		{
			exit(0);
		}

	}
	
}


//Video oynarken mouse ile üzerinde yapýlan iþlemler
void mouseHandler(int event, int x, int y, int flags, void *param)
{
	//Mouse sað klik týklandýðýnda video durdurulur
	if (event == CV_EVENT_RBUTTONDOWN && !drag)
	{
		playVideo = !playVideo;
	}

	//Mouse sol klik týklandýðýnda çizdiðimiz dikdörtgenin point1 noktasý oluþur
	if (event == CV_EVENT_LBUTTONDOWN && !drag)
	{

		point1 = Point(x, y);
		drag = 1;
		cout << "point1.x..:" << point1.x << endl << "point1.y..: " << point1.y << endl;
	}

	//Mouse sol klik basýlýyken oynattýðýmýzda o koordinatta kutuyu oluþturur
	if (event == CV_EVENT_MOUSEMOVE && drag)
	{

		Mat img1 = frame.clone();
		point2 = Point(x, y);
		rectangle(img1, point1, point2, CV_RGB(255, 0, 0), 3, 8, 0);
		imshow(image_window, img1);
	}

	//Mouse sol klik kaldýrýldýðýnda þablon oluþturulur
	if (event == CV_EVENT_LBUTTONUP && drag)
	{
		point2 = Point(x, y);
		//Þablon çok küçük olduðunda video durur
		if (x-point1.x<10 || y-point1.y<10)
			playVideo = false;

		if(y < point1.y && x > point1.x) //Videoda kutuyu oluþtururken sol alttan sað üste doðru oluþturduðumuzda çalýþýr
			rect = Rect(point1.x, y, x - point1.x, point1.y - y);
		else if (y > point1.y && x < point1.x) //Videoda kutuyu oluþtururken sað üstten sol aþaðý doðru oluþturduðumuzda çalýþýr
			rect = Rect(x, point1.y, point1.x-x, y-point1.y);
		else if (y < point1.y && x < point1.x) //Videoda kutuyu oluþtururken sað alttan sol üste doðru oluþturduðumuzda çalýþýr
			rect = Rect(x, y, point1.x-x, point1.y - y);
		else  //Videoda kutuyu oluþtururken sol üstten sað aþaðý doðru oluþturduðumuzda çalýþýr
			rect = Rect(point1.x, point1.y, (x - point1.x) , (y - point1.y) );

		drag = 0;

		roiImg = frame(rect);
		roiImg.copyTo(templ); // Seçtiðimiz kutuyu "templ" adýndaki þablon fotoðrafý tutan deðiþkene atar ve videoda onu aramaya baþlar

	}

	//Mouse orta klik týklandýðýnda video kapanýr ve menüye döner
	if (event == CV_EVENT_MBUTTONDOWN)
	{
		
		destroyWindow(image_window);
		destroyWindow(result_window);
		waitKey();

	}

}

//Menü oluþturuluyor
void Menu()
{

	//Menüdeki dikdörtgenler çizer
	rectangle(img, Point(0, 0), Point(250, 50), CV_RGB(169, 169, 169), -1, 8, 0);
	rectangle(img, Point(0, 50), Point(250, 100), CV_RGB(30, 144, 255), -1, 8, 0);
	rectangle(img, Point(0, 100), Point(250, 150), CV_RGB(0, 255, 0), -1, 8, 0);
	rectangle(img, Point(0, 150), Point(250, 200), CV_RGB(220, 20, 60), -1, 8, 0);
	
	//Menüdeki yazýlar ekler
	putText(img, buttonText, Point(80, 30), FONT_HERSHEY_PLAIN, 1, Scalar(0, 0, 0));
	putText(img, buttonText2, Point(12, 80), FONT_HERSHEY_PLAIN, 1, Scalar(0, 0, 0));
	putText(img, buttonText3, Point(90, 130), FONT_HERSHEY_PLAIN, 1, Scalar(0, 0, 0));
	putText(img, buttonText4, Point(100, 180), FONT_HERSHEY_PLAIN, 1, Scalar(0, 0, 0));



	namedWindow(winName); // Pencere ismini verir
	setMouseCallback(winName, callBackFunc); // Menüde yaptýðýmýz iþlemlerin fonksiyonu

	imshow(winName, img); //Menüyü ekrana getirir
	waitKey();
}

int Run()
{
	
	string fileName = vPath; //videonun yolu fileName deðiþkeninde tutulur
	VideoCapture capture(fileName); // capture deðiþkeni videoyu alýr

	string imgDir = templPath; // Þablon fotoðrafýn yolu "imgDir" deðiþkeninde tutulut
	//Þablon Fotoðraf seçilmemiþse boþ beyaz bir sayfa varsayýlan olarak ayarlanýr
	if (imgDir == "")
	{
		Mat img_temp(50, 50, CV_8UC3, Scalar(255, 255, 255));
		templ = img_temp;
	}

	else
		templ = imread(imgDir, IMREAD_COLOR); // þablon resmi templ deðiþkenine atar

	//Video okunmadýðýnda hata yollar
	if (!capture.isOpened()) 
		throw "Video okunurken hata";

	//Pencere ismi verilir
	namedWindow(image_window, WINDOW_AUTOSIZE);

	//Framelerin peþpeþe geldiði sonsuz döngü oluþturulur
	for (; ; )
	{
		//Video durdurulmadýysa framler peþpeþe gelmeye devam eder
		if (playVideo)
			capture >> frame;

		//video bittiyse sonsuz döngüden çýkýlýr
		if (frame.empty())
			break;

		//Videonun üstünde bar içindeki metodlar
		const char* trackbar_label = "Metod   ";
		createTrackbar(trackbar_label, image_window, &match_method, max_Trackbar, MatchingMethod);
		//Video içinde þablon seçmek istersek fonksiyon çalýþýr
		cvSetMouseCallback(image_window, mouseHandler, NULL);
		MatchingMethod(0, 0); // Eþleþtirme metodu
		waitKey(10); // frame bekleme süresi

	}
	//Video bittiðinde pencereler kapatýlýr ve menüye döner
	destroyWindow(image_window);
	destroyWindow(result_window);
	waitKey();

}


//Þablon görüntüyle frame'de eþleþen (veya benzer) alaný bulmak için kullanýlan metod
void MatchingMethod(int, void*)
{
	Mat img_display;
	frame.copyTo(img_display);
	int result_cols = frame.cols - templ.cols + 1;
	int result_rows = frame.rows - templ.rows + 1;

	matchTemplate(frame, templ, result, match_method); // Seçilen nesneyi arayan fonksiyon

	normalize(result, result, 0, 1, NORM_MINMAX, -1, Mat());
	double minVal; double maxVal; Point minLoc; Point maxLoc;
	Point matchLoc;

	//Minimum ve maksimum eleman deðerlerini ve konumlarýný bulur
	minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, Mat());
	if (match_method == TM_SQDIFF || match_method == TM_SQDIFF_NORMED)
	{
		matchLoc = minLoc;
	}
	else
	{
		matchLoc = maxLoc;
	}
	//Þablonn eþleþtiði bölge kutu içine alýnýr
	rectangle(img_display, matchLoc, Point(matchLoc.x + templ.cols, matchLoc.y + templ.rows), Scalar::all(0), 2, 8, 0);
	//Sonuç penceresinde þablonun eþleþtiði bölge kutu içine alýnýr
	rectangle(result, matchLoc, Point(matchLoc.x + templ.cols, matchLoc.y + templ.rows), Scalar::all(0), 2, 8, 0);
	imshow(result_window, result); // Video penceresi ekrana getirilir
	imshow(image_window, img_display); // Sonuç penceresi ekrana getirilir
	return;
}

int main(int argc, char** argv)
{
	HideConsole();

	Menu();

	return 0;
}

