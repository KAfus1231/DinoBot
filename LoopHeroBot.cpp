#include <Windows.h>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <vector>
#include <iostream>
#include <chrono>

#define GREEN "\033[32m"
#define RED "\033[31m"
#define STANDART "\033[0m"

#define WIDTH 760 
#define HEIGHT 175

struct Config
{
	int maxDist = 80;
	int delayInput = 110;
	float calibrationFactor = 10.0f;

	cv::Point dinoPoint{ 50, 50 };
	double minCactusArea = 300;
	double maxCactusArea = 1000;

	float minJumpY = 90;
	float maxJumpHeight = 160;

	struct DayNightHSV
	{
		cv::Scalar minHSV;
		cv::Scalar maxHSV;
	};

	DayNightHSV dayHSV = { cv::Scalar(0, 0, 140), cv::Scalar(0, 0, 180) };
	DayNightHSV nightHSV = { cv::Scalar(0, 0, 100), cv::Scalar(0, 0, 255) };
};

void on_trackbar(int, void*) {}

cv::Mat hwnd2mat(HWND hwnd)
{
	cv::Mat src(HEIGHT, WIDTH, CV_8UC4);
	HDC hwindowDC = GetDC(NULL);
	HDC hwindowCompatibleDC = CreateCompatibleDC(hwindowDC);

	HBITMAP hbwindow = CreateCompatibleBitmap(hwindowDC, WIDTH, HEIGHT);

	BITMAPINFOHEADER bi;
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = WIDTH;
	bi.biHeight = -HEIGHT;
	bi.biPlanes = 1;
	bi.biBitCount = 32;
	bi.biCompression = BI_RGB;

	SelectObject(hwindowCompatibleDC, hbwindow);
	StretchBlt(
		hwindowCompatibleDC,
		0, 0,
		WIDTH, HEIGHT,
		hwindowDC,
		580, 150,
		WIDTH, HEIGHT,
		SRCCOPY);

	GetDIBits(
		hwindowCompatibleDC,
		hbwindow,
		0,
		HEIGHT,
		src.data,
		(BITMAPINFO*)&bi,
		DIB_RGB_COLORS);

	DeleteObject(hbwindow);
	DeleteDC(hwindowCompatibleDC);
	ReleaseDC(hwnd, hwindowDC);

	return src;
}

void templateSearch(cv::Mat temp1, cv::Mat& img)
{
	cv::Mat result;

	float res_cols = img.cols - temp1.cols + 1;
	float res_rows = img.rows - temp1.rows + 1;

	result.create(res_rows, res_cols, CV_32FC1);

	cv::matchTemplate(img, temp1, result, cv::TM_CCOEFF_NORMED);

	double minVal, maxVal;
	cv::Point minLoc, maxLoc;

	cv::minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc);

	if(maxVal >= 0.8)
		cv::rectangle(img, maxLoc, cv::Point(maxLoc.x + temp1.cols, maxLoc.y + temp1.rows), cv::Scalar(0, 255, 255), 2);

}

std::atomic<int> runTime{0};
void runTimer()
{
	std::thread([]
		{
			while (true)
			{
				std::this_thread::sleep_for(std::chrono::seconds(1));
				runTime++;
			}

		}).detach();
}

void pressSpace(int delayInput, bool &timerIsWorking)
{
	if (!timerIsWorking)
	{
		timerIsWorking = true;
		runTimer();
		std::cout << "time is work!" << std::endl;
	}
	std::thread([delayInput] {
		INPUT input[1] = {};

		//НАЖАТИЕ
		input[0].type = INPUT_KEYBOARD;
		input[0].ki.wVk = VK_SPACE;
		SendInput(1, input, sizeof(INPUT));

		std::this_thread::sleep_for(std::chrono::milliseconds(delayInput));

		//ОТПУСКАНИЕ
		input[0].ki.dwFlags = KEYEVENTF_KEYUP;
		SendInput(1, input, sizeof(INPUT));
		}).detach();
}

void pressDown(int delayInput)
{
	std::thread([delayInput]
		{
			INPUT input[1] = {};

			input[0].type = INPUT_KEYBOARD;
			input[0].ki.wVk = VK_DOWN;
			SendInput(1, input, sizeof(INPUT));

			std::this_thread::sleep_for(std::chrono::milliseconds(delayInput));

			input[0].ki.dwFlags = KEYEVENTF_KEYUP;
			SendInput(1, input, sizeof(INPUT));
		}).detach();
}
double getBrightness(cv::Mat img)
{
	cv::Mat grayMask;
	cv::cvtColor(img, grayMask, cv::COLOR_BGR2GRAY);
	double brightness = cv::mean(grayMask)[0];
	
	return brightness;
}

void detectCactus(cv::Mat& img, cv::Scalar lowHSV, cv::Scalar highHSV,
	int maxDist, int delayInput, bool &timerIsWorking)
{
	Config config;

	cv::Mat hsv, mask;
	cv::cvtColor(img, hsv, cv::COLOR_BGR2HSV);

	cv::inRange(hsv, lowHSV, highHSV, mask);

	cv::erode(mask, mask, cv::Mat(), cv::Point(-1, -1), 1);
	cv::dilate(mask, mask, cv::Mat(), cv::Point(-1, -1), 2);

	std::vector<std::vector<cv::Point>> contours;
	cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

	for (const auto& contour : contours)
	{
		double area = cv::contourArea(contour);
		if (area > config.minCactusArea && area < config.maxCactusArea)
		{
			cv::Rect rect = cv::boundingRect(contour);
			cv::rectangle(img, rect, cv::Scalar(0, 255, 255), 2);

			int distance = rect.x - config.dinoPoint.x;

			if (distance < maxDist && rect.y > config.minJumpY)
			{
				if (rect.y + rect.height < config.maxJumpHeight)
					pressDown(delayInput * 6);
				else
					pressSpace(delayInput, timerIsWorking);
			}
		}
	}
}

void trackbars()
{
	/*int lowH = 0, highH = 180;
	int lowS = 0, highS = 255;
	int lowV = 0, highV = 255;*/
	
	/*cv::createTrackbar("Distance", "Dino", &maxDist, 150, on_trackbar);
	cv::createTrackbar("Delay Input", "Dino", &delayInput, 150, on_trackbar);*/

	/*cv::createTrackbar("Low H", "Dino", &lowH, 180, on_trackbar);
	cv::createTrackbar("High H", "Dino", &highH, 180, on_trackbar);

	cv::createTrackbar("Low S", "Dino", &lowS, 255, on_trackbar);
	cv::createTrackbar("High S", "Dino", &highS, 255, on_trackbar);

	cv::createTrackbar("Low V", "Dino", &lowV, 255, on_trackbar);
	cv::createTrackbar("High V", "Dino", &highV, 255, on_trackbar);*/
}

int main() 
{
	cv::namedWindow("Dino", cv::WINDOW_NORMAL);
	HWND hwndDesktop = FindWindow(NULL, L"Microsoft Edge");
	Config config;

	bool timerIsWorking = false;
	int lastTimeUpdate = 0;

	while(true)
	{
		cv::Mat img = hwnd2mat(hwndDesktop);
		cv::putText(img, "Delay: " + std::to_string(config.delayInput) + " MaxDist: " +
			std::to_string(config.maxDist), cv::Point(50, 50), cv::FONT_HERSHEY_SIMPLEX, 1.0,
			cv::Scalar(255, 255, 255), 1);

		if (getBrightness(img) > 150)
		{
			detectCactus(img, config.dayHSV.minHSV, config.dayHSV.maxHSV, config.maxDist, config.delayInput, timerIsWorking); // 140 180
		}
		else
		{
			detectCactus(img, config.nightHSV.minHSV, config.nightHSV.maxHSV, config.maxDist, config.delayInput, timerIsWorking); // 100 255
		}

		if (runTime.load() > 6 && runTime.load() % 7 == 0 && lastTimeUpdate != runTime.load())
		{
			lastTimeUpdate = runTime.load();

			if (config.maxDist < 150)
				config.maxDist += config.calibrationFactor;

			if (config.delayInput > 50)
				config.delayInput -= config.calibrationFactor;
		}

		cv::imshow("Dino", img);
		if (cv::waitKey(1) == 27) break;
	}

	return 0;
}


