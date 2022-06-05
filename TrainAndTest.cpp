// TrainAndTest.cpp

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/ml/ml.hpp>

#include<iostream>
#include<sstream>

// global variables ///////////////////////////////////////////////////////////////////////////////
const int MIN_CONTOUR_AREA = 100;

const int RESIZED_IMAGE_WIDTH = 20;
const int RESIZED_IMAGE_HEIGHT = 30;


cv::VideoCapture cap(0);

//int Search4Logo(Mat matTestingNumbers,std::string sFileClass,std::string sFileImage,char* sFinal);
int MyReconChar(cv::Mat,cv::Ptr<cv::ml::KNearest>,char*);
///////////////////////////////////////////////////////////////////////////////////////////////////
class ContourWithData {
public:
    // member variables ///////////////////////////////////////////////////////////////////////////
    std::vector<cv::Point> ptContour;            
    cv::Rect boundingRect;                       
    float fltArea;                               

                                                ///////////////////////////////////////////////////////////////////////////////////////////////
    bool checkIfContourIsValid() {                               
        if (fltArea < MIN_CONTOUR_AREA) return false;           
        return true;                                            
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    static bool sortByBoundingRectXPosition(const ContourWithData& cwdLeft, const ContourWithData& cwdRight) {       
        return(cwdLeft.boundingRect.x < cwdRight.boundingRect.x);                                                    
    }

};

int Search4Logo(cv::Mat matTestingNumbers,std::string sFileClass,std::string sFileImage,char* sFinal)
{


    return 0;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
int main() {


                                                                 

    cv::Mat matClassificationInts;       

    cv::FileStorage fsClassifications("classifications.xml", cv::FileStorage::READ);         

    if (fsClassifications.isOpened() == false) {                                                     
        std::cout << "error, unable to open training classifications file, exiting program\n\n";     
        return(0);                                                                                   
    }

    fsClassifications["classifications"] >> matClassificationInts;       
    fsClassifications.release();                                         
                                                                         

    cv::Mat matTrainingImagesAsFlattenedFloats;          

    cv::FileStorage fsTrainingImages("images.xml", cv::FileStorage::READ);          

    if (fsTrainingImages.isOpened() == false) {                                                  
        std::cout << "error, unable to open training images file, exiting program\n\n";          
        return(0);                                                                               
    }

    fsTrainingImages["images"] >> matTrainingImagesAsFlattenedFloats;            
    fsTrainingImages.release();                                                  

                                                                                   

    cv::Ptr<cv::ml::KNearest>  kNearest(cv::ml::KNearest::create());             

                                                                                
    kNearest->train(matTrainingImagesAsFlattenedFloats, cv::ml::ROW_SAMPLE, matClassificationInts);

      

    //cv::Mat matTestingNumbers = cv::imread("test1.png");            
    char strFinal[255]={'\0'};
    cv::Mat matTestingNumbers;
    cv::Mat HSV;
    cv::Mat BGR2HSV;


    while (true)
    {
        //matTestingNumbers.zeros()
		cap.read(matTestingNumbers);
            if (matTestingNumbers.empty()) {                                 
                std::cout << "error: image not read from file\n\n";         
                return(0);                                                  
            }


    cv::cvtColor(matTestingNumbers,BGR2HSV, cv::COLOR_BGR2HSV);
    //Pt Beko
    ///cv::inRange(BGR2HSV, cv::Scalar(0, 0, 0), cv::Scalar(53, 255, 255), HSV);

    //Pt Lamona
    cv::inRange(BGR2HSV, cv::Scalar(0, 30, 130), cv::Scalar(23,255,255), HSV);

        char pkey=0;
        pkey=cv::waitKey(1);
        //pkey=13;
        if (pkey==13)
        {// wait for user key press
MY_Label:

            memset(strFinal,'\0',255);
            MyReconChar(HSV,kNearest,strFinal);
            cv::putText(matTestingNumbers, strFinal, cv::Point(50,200), cv::FONT_HERSHEY_PLAIN, 2.5, cv::Scalar(255,0,0,255), 3,8);
            std::cout << "\n\n" << "numbers read = " << strFinal << "\n\n";       
            cv::Rect xx=cv::boundingRect(HSV);
            cv::rectangle(matTestingNumbers,xx,cv::Scalar(0, 255, 0),2);


        }
        else if (pkey ==115)
        {
            //cv::Mat HSVNOT;
            std::vector<int> compression_params;
            compression_params.push_back(cv::IMWRITE_PNG_COMPRESSION);
            compression_params.push_back(9);
            bool result = false;
            try
            {
                result = imwrite("lamonanot.png", HSV, compression_params);
            }
            catch (const cv::Exception& ex)
            {
                fprintf(stderr, "Exception converting image to PNG format: %s\n", ex.what());
            }
            imshow ("Save",HSV);
        }
        else if(pkey==27)
        {
            exit(0);
        }
        cv::imshow("matTestingNumbers", matTestingNumbers);     
        cv::imshow("Filter HSV",HSV);
	}





    return(0);
}
int MyReconChar(cv::Mat matTestingNo,cv::Ptr<cv::ml::KNearest>  kNearest,char* sBuff)
{
    std::vector<ContourWithData> allContoursWithData;           
    std::vector<ContourWithData> validContoursWithData;         
    std::string strFinalString;         
    cv::Mat matGrayscale;           
    cv::Mat matBlurred;             
    cv::Mat matThresh;             
    cv::Mat matThreshCopy;         

    cv::GaussianBlur(matTestingNo,           
        matBlurred,                
        cv::Size(5, 5),            
        0);                        


    cv::adaptiveThreshold(matBlurred,                           
        matThresh,                            
        255,                                  
        cv::ADAPTIVE_THRESH_GAUSSIAN_C,       
        cv::THRESH_BINARY_INV,                
        11,                                   
        2);                                   

    matThreshCopy = matThresh.clone();              

    std::vector<std::vector<cv::Point> > ptContours;        
    std::vector<cv::Vec4i> v4iHierarchy;                    

    cv::findContours(matThreshCopy,             
        ptContours,                             
        v4iHierarchy,                           
        cv::RETR_EXTERNAL,                      
        cv::CHAIN_APPROX_SIMPLE);               

    for (int i = 0; i < (int)ptContours.size(); i++) {              
        ContourWithData contourWithData;                                                    
        contourWithData.ptContour = ptContours[i];                                          
        contourWithData.boundingRect = cv::boundingRect(contourWithData.ptContour);         

            contourWithData.fltArea = cv::contourArea(contourWithData.ptContour);               
            allContoursWithData.push_back(contourWithData);
        
    }

    for (int i = 0; i < allContoursWithData.size(); i++) {                      
        if (allContoursWithData[i].checkIfContourIsValid()) {                   // check if valid
            validContoursWithData.push_back(allContoursWithData[i]);            // if so, append to valid contour list
        }
    }
    
    std::sort(validContoursWithData.begin(), validContoursWithData.end(), ContourWithData::sortByBoundingRectXPosition);



    for (int i = 0; i < validContoursWithData.size(); i++) {            

                                                                        
        cv::rectangle(matTestingNo,                            
            validContoursWithData[i].boundingRect,        
            cv::Scalar(0, 255, 0),                        
            2);                                          

        cv::Mat matROI = matThresh(validContoursWithData[i].boundingRect);          

        cv::Mat matROIResized;
        cv::resize(matROI, matROIResized, cv::Size(RESIZED_IMAGE_WIDTH, RESIZED_IMAGE_HEIGHT));     

        cv::Mat matROIFloat;
        matROIResized.convertTo(matROIFloat, CV_32FC1);             

        cv::Mat matROIFlattenedFloat = matROIFloat.reshape(1, 1);

        cv::Mat matCurrentChar(0, 0, CV_32F);

        kNearest->findNearest(matROIFlattenedFloat, 1, matCurrentChar);     

        float fltCurrentChar = (float)matCurrentChar.at<float>(0, 0);

        strFinalString = strFinalString + char(int(fltCurrentChar));       

    }
    const char* p_data  = strFinalString.data();
    strcpy(sBuff,p_data);
    return 0;

}

