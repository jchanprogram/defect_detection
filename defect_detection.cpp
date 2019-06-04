#include <stdio.h>
#include <math.h>
#include <iostream>
#include <string>
#include <ctime>

#include <opencv2/opencv.hpp>

/******************************************************************************************************************************************************************/
//	La première partie permet de détecter un "défaut" : le défaut est une tâche sombre sur un fond de niveau de gris d'environ 120
//	Pour cela, un seuillage inverse est réalisé pour détecter le défaut
//	Les fonctions erode et dilate sont utilisées pour traiter le défaut afin de déterminer le contour
//	La compacite et la surface du défaut sont calculer
//	Un rectangle encadrant le défaut est calculer

// 	Dans la seconde partie on va filmer une scène afin d'isoler un objet d'une certaine couleur (HSV) choisie par 3 trackbars
//	Cet objet est ensuite traitée (seuil, erode, dilate)

int main(int argc, char** argv)
{
	
	cv::String pathname = "../defaut/";
	cv::String filename = "blackhole.jpg";
	

	cv::Mat imageDefaut_temp = cv::imread(pathname + filename);
	cv::Mat imageDefaut;
	cv::cvtColor(imageDefaut_temp, imageDefaut, CV_BGR2GRAY);
	std::cout << "channel image primaire = " << imageDefaut.channels() << std::endl;

	cv::String windowNameDefaut = "Image defaut";
	namedWindow(windowNameDefaut);
	cv::moveWindow(windowNameDefaut, 10, 10);
	
	imshow(windowNameDefaut, imageDefaut);

//Create trackbars in "Control" window in we need to ajust the treshhold

	//int seuil = 100;
	//cv::namedWindow("Seuil Control", CV_WINDOW_AUTOSIZE); //create a window called "Control"
		

//	cv::namedWindow("Seuil Control");
//	cv::createTrackbar("Seuil", "Seuil Control", &seuil, 255);
//	while (true)
//	{"operation on image" }
	int seuil = 110;

		cv::Mat imageDefautThreshold;
		cv::threshold(imageDefaut, imageDefautThreshold, seuil, 255, cv::THRESH_BINARY_INV);

		cv::String windowNameDefautThreshold = "Image defaut seuillee";
		namedWindow(windowNameDefautThreshold);
		imshow(windowNameDefautThreshold, imageDefautThreshold);


		cv::Mat imageDefautErode;
		cv::erode(imageDefautThreshold, imageDefautErode, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3)));

		cv::Mat imageDefautDilate;
		cv::dilate(imageDefautThreshold, imageDefautDilate, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5)));
		cv::Mat imageDefautDilateMinusErode = imageDefautDilate - imageDefautErode; // Contour of defect

		cv::String windowNameDefautErode = "Image defaut contour dilate moins erode";
		namedWindow(windowNameDefautErode);
		cv::moveWindow(windowNameDefautErode, 610, 10);
		
		imshow(windowNameDefautErode, imageDefautDilateMinusErode);


		cv::Mat imageDefautDilateAndErode;
		cv::erode(imageDefautDilate, imageDefautDilateAndErode, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5))); // remove all the black pixel inside the defect

		cv::String windowNameDefautDilateAndErode = "Image defaut dilate and erode";
		namedWindow(windowNameDefautDilateAndErode);

		imshow(windowNameDefautDilateAndErode, imageDefautDilateAndErode);

		

		//////////////////////////////////////////////////////////////////////////////////////////////////////
		// use find contours on image "dilate and erode" and show defect and contour
		
		cv::Scalar color = cv::Scalar(255, 100, 100);
		std::vector<std::vector<cv::Point>> contours;
		std::vector<cv::Vec4i> hierarchy;
		
		cv::findContours(imageDefautDilateAndErode, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);// , cv::Point(0, 0));
		cv::Mat drawcontour = cv::Mat::zeros(imageDefautDilateAndErode.size(), CV_8UC3);
		
		drawContours(drawcontour, contours, -1, color, 1, 8, hierarchy);
		cv::String windowNameDefautfindcontour = "Find contour";
		namedWindow(windowNameDefautfindcontour);
		imshow(windowNameDefautfindcontour, drawcontour);

		///////////////////////////////////////////////////////////////////////////////////////////////////////

		int compteur = 0;
		compteur = cv::countNonZero(imageDefautDilateAndErode);
		std::cout << "Surface" << compteur << "\n" << std::endl;
		int compteurTresh = 0;
		compteurTresh = cv::countNonZero(imageDefautThreshold);
		std::cout << "Surface Tresh" << compteurTresh << "\n" << std::endl;

		///////////////////////////////////////////////////////////////////////////////////////////////////////
		/* Find the contour by using the image with all remove black pixel and not using the function find contour*/
		cv::Mat imageDefautDilateAndErodeErode;
		cv::erode(imageDefautDilateAndErode, imageDefautDilateAndErodeErode, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3)));
		imageDefautDilateAndErodeErode = imageDefautDilateAndErode - imageDefautDilateAndErodeErode;

		cv::String windowNameDefautRechercheContour = "Image defaut recherche contour";
		namedWindow(windowNameDefautRechercheContour);

		cv::moveWindow(windowNameDefautRechercheContour, 10, 410);
		imshow(windowNameDefautRechercheContour, imageDefautDilateAndErodeErode);

		cv::Mat imageMasque;
		

		imageMasque = imageDefautDilateAndErodeErode + imageDefaut;
		cv::String windowNameDefautMasque = "Image defaut avec masque";
		namedWindow(windowNameDefautMasque);
		cv::moveWindow(windowNameDefautMasque, 410, 410);
		imshow(windowNameDefautMasque, imageMasque);


		/* recherche du rectangle incluant le défaut dans le mat ayant le contour*/

		cv::Mat positionContour;
		imageDefautDilateAndErodeErode.copyTo(positionContour); //binary image

		std::vector<cv::Point> locations;   // output, locations of non-zero pixels
		cv::findNonZero(positionContour, locations);
		
		int compteur2(0);
		
		compteur2 = cv::countNonZero(imageDefautDilateAndErode);
		std::cout << "Surface du defaut" << compteur2 << "\n" << std::endl;
		std::cout << "Nombre de pixels du contour " << locations.size() << std::endl;


		cv::Point minLargeur, maxLargeur, minLongueur, maxLongueur;
		
		if (locations.size() > 10) // the limit remove very little defect
		{
			minLargeur = *std::min_element(locations.begin(), locations.end(), [](cv::Point & point1, cv::Point & point2)
			{
				return point1.x < point2.x;
			});

			maxLargeur = *std::max_element(locations.begin(), locations.end(), [](cv::Point & point1, cv::Point & point2)
			{
				return point1.x < point2.x;
			});

			minLongueur = *std::min_element(locations.begin(), locations.end(), [](cv::Point & point1, cv::Point & point2)
			{
				return point1.y < point2.y;
			});
			maxLongueur = *std::max_element(locations.begin(), locations.end(), [](cv::Point & point1, cv::Point & point2)
			{
				return point1.y < point2.y;
			});
		}

		cv::rectangle(imageMasque, cv::Point(minLargeur.x, minLongueur.y), cv::Point(maxLargeur.x, maxLongueur.y), cv::Scalar(255), 1, cv::LINE_8);
		imshow(windowNameDefautMasque, imageMasque);
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		// Compacite 
		float compacite;
		int surface = cv::countNonZero(imageDefautDilateAndErode);
		compacite = (float)surface / ((maxLargeur.x - minLargeur.x + 1)*(maxLongueur.y - minLongueur.y + 1));
		std::cout << "Surface = " << surface << std::endl;
		std::cout << "Surface rectangle = " << (maxLargeur.x - minLargeur.x + 1)*(maxLongueur.y - minLongueur.y + 1) << std::endl;
		std::cout << "compacite est de " << compacite << std::endl;


	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/* CAPTURE VIDEO AVEC LA WEBCAM*/
	/* Détection de couleur sur une video en utilisant la fonction inRange */
	/* Utilisation de Trackbar pour changer les valeurs HSV */
	/* Traitement de l'image video par seuillage, erode, dilatation afin de trouver le contour de l'objet de la couleur souihaité*/
	cv::VideoCapture cap(0);
	int seuil2 = 125;

	// if not success, exit program
	if (cap.isOpened() == false)
	{
		std::cout << "Cannot open the video camera" << std::endl;
		std::cin.get(); //wait for any key press
		return -1;
	}

	double dWidth = cap.get(cv::CAP_PROP_FRAME_WIDTH); //get the width of frames of the video
	double dHeight = cap.get(cv::CAP_PROP_FRAME_HEIGHT); //get the height of frames of the video

	std::cout << "Resolution of the video : " << dWidth << " x " << dHeight << std::endl;
	
	std::string window_name = "My Camera";
	cv::namedWindow(window_name); //create a window called "My Camera Feed"

	/////////////////////////////////////////////////////////////////////////////////////////////////

	cv::namedWindow("Control", CV_WINDOW_AUTOSIZE); //create a window called "Control"

	int iLowH = 0;
	int iHighH = 179;

	int iLowS = 0;
	int iHighS = 255;

	int iLowV = 0;
	int iHighV = 255;

	//Create trackbars in "Control" window
	cvCreateTrackbar("LowH", "Control", &iLowH, 179); //Hue (0 - 179)
	cvCreateTrackbar("HighH", "Control", &iHighH, 179);

	cvCreateTrackbar("LowS", "Control", &iLowS, 255); //Saturation (0 - 255)
	cvCreateTrackbar("HighS", "Control", &iHighS, 255);

	cvCreateTrackbar("LowV", "Control", &iLowV, 255); //Value (0 - 255)
	cvCreateTrackbar("HighV", "Control", &iHighV, 255);
	/////////////////////////////////////////////////////////////////////////////////////////////////

	while (true)
	{
		cv::Mat frame,frame2;
		bool bSuccess = cap.read(frame); // read a new frame from video 

		//Breaking the while loop if the frames cannot be captured
		if (bSuccess == false)
		{
			std::cout << "Video camera is disconnected" << std::endl;
			std::cin.get(); //Wait for any key press
			break;
		}

		/////////////////////////////////////////////////////////////////////////////////////////////////
		cv::Mat imgHSV;
		cvtColor(frame, imgHSV, cv::COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV

		cv::Mat imgThresholded;

		inRange(imgHSV, cv::Scalar(iLowH, iLowS, iLowV), cv::Scalar(iHighH, iHighS, iHighV), imgThresholded); //Threshold the image

		//morphological opening (remove small objects from the foreground)
		erode(imgThresholded, imgThresholded, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3)));
		dilate(imgThresholded, imgThresholded,cv:: getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3)));

		//morphological closing (fill small holes in the foreground)
		dilate(imgThresholded, imgThresholded, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3)));
		erode(imgThresholded, imgThresholded, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3)));

		imshow("Thresholded Image", imgThresholded); //show the thresholded image
		imshow("Original", frame); //show the original image



		//show the frame in the created window
		imshow(window_name, frame);
		cap.read(frame2);
		cv::cvtColor(frame2, frame2, CV_BGR2GRAY);
		cv::Mat imageDefautThresholdFrame;
		cv::threshold(frame2, imageDefautThresholdFrame, seuil2, 255, cv::THRESH_BINARY);

		cv::Mat imageDefautErodeFrame;
		cv::erode(imageDefautThresholdFrame, imageDefautErodeFrame, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3)));

		cv::Mat imageDefautDilateFrame;
		cv::dilate(imageDefautThresholdFrame, imageDefautDilateFrame, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5)));
		cv::Mat imageDefautDilateMinusErodeFrame = imageDefautDilateFrame - imageDefautErodeFrame;


		cv::Mat imageDefautDilateAndErodeFrame;
		cv::erode(imageDefautDilateFrame, imageDefautDilateAndErodeFrame, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5)));

		
		cv::Mat imageDefautDilateAndErodeErodeFrame;
		cv::erode(imageDefautDilateAndErodeFrame, imageDefautDilateAndErodeErodeFrame, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3)));
		imageDefautDilateAndErodeErodeFrame = imageDefautDilateAndErodeFrame - imageDefautDilateAndErodeErodeFrame;

		cv::Mat imageMasqueFrame;
		

		imageMasqueFrame = imageDefautDilateAndErodeErodeFrame + frame2;
		cv::String windowNameDefautMasqueFrame = "Video traitee";
		namedWindow(windowNameDefautMasqueFrame);

		imshow(windowNameDefautMasqueFrame, imageMasqueFrame);

		/////////////////////////////////////////////////////////////////////////////////////////////////


		if (cv::waitKey(10) == 27)
		{
			std::cout << "Esc key is pressed by user. Stoppig the video" << std::endl;
			break;
		}
		
	}


	return 0;
}
