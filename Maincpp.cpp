//--------------------------------------------------------------------
//				Bingyang Liu  2015/07/14
//				Meanshift Algorithm
//				OpenCV 3.0.0
//--------------------------------------------------------------------

//---------------- Head  File ---------------------------------------
#include <iostream>
#include "MeanShift.h"
#include "Timer.h"

#define WIDTH_SMALL     160
#define HEIGHT_SMALL    120


typedef struct _Input 
{
    _Input(int sb, int cb, float ra, float rl)
    {
        m_spatial_bandwidth = sb;   m_color_bandwidth = cb; m_ratio_area = ra;  m_ratio_length = rl; 
        m_sb_init = sb;             m_cb_init = cb;         m_ra_init = ra;
        //m_ms = MeanShift(m_sb_init, m_cb_init);
    }
    Mat m_im_bgr;
    //Size m_sz_small;
    int m_spatial_bandwidth, m_color_bandwidth, m_sb_init, m_cb_init;
    float m_ratio_area, m_ra_init, m_ratio_length;
    //MeanShift m_ms;
    vector<Scalar> m_li_bgr;
} Input;

typedef struct _Output 
{
    /*
        _Output(const Mat& im_bgr)
            {
                    im_bgr_rot_center = im_bgr.clone();
                        }
                            */
    Mat m_im_segmented;
} Output;




int g_spatial_max = 30, g_color_max = 100, g_area_max = 400;

static void help(char** argv)
{
    cout << "\nDemonstrate mean-shift based color segmentation in spatial pyramid.\n"
    << "Call:\n   " << argv[0] << " image spatial_bandwidth color_bandwidth th_area\n"
    << "This program allows you to set the spatial and color radius\n"
    << "of the mean shift window as well as the number of pyramid reduction levels explored\n"
    << endl;
}


inline int random_number_in_between(int minV, int maxV)
{
    return rand() % (maxV - minV + 1) + minV;
    }    


vector<Scalar> generate_random_color_list(unsigned int n_color)
{
    vector<Scalar> li_color;
        li_color.push_back(Scalar(255, 0, 0));
            li_color.push_back(Scalar(0, 255, 0));
                li_color.push_back(Scalar(0, 0, 255));
                    if(li_color.size() < n_color)
                        {
                                li_color.push_back(Scalar(255, 255, 0));
                                        li_color.push_back(Scalar(0, 255, 255));
                                                li_color.push_back(Scalar(255, 0, 255));
                                                        if(li_color.size() < n_color)
                                                                {
                                                                            li_color.push_back(Scalar(255, 128, 0));
                                                                                        li_color.push_back(Scalar(0, 255, 128));
                                                                                                    li_color.push_back(Scalar(128, 0, 255));
                                                                                                                if(li_color.size() < n_color)
                                                                                                                            {
                                                                                                                                            li_color.push_back(Scalar(128, 255, 0));
                                                                                                                                                            li_color.push_back(Scalar(0, 128, 255));
                                                                                                                                                                            li_color.push_back(Scalar(255, 0, 128));
                                                                                                                                                                                            if(li_color.size() < n_color)
                                                                                                                                                                                                            {
                                                                                                                                                                                                                                li_color.push_back(Scalar(128, 128, 0));
                                                                                                                                                                                                                                                    li_color.push_back(Scalar(0, 128, 128));
                                                                                                                                                                                                                                                                        li_color.push_back(Scalar(128, 0, 128));
                                                                                                                                                                                                                                                                                            
                                                                                                                                                                                                                                                                                                                while(li_color.size() < n_color)
                                                                                                                                                                                                                                                                                                                                    {
                                                                                                                                                                                                                                                                                                                                                            li_color.push_back(Scalar(
                                                                                                                                                                                                                                                                                                                                                                                        random_number_in_between(100, 255),
                                                                                                                                                                                                                                                                                                                                                                                                                    random_number_in_between(100, 255),
                                                                                                                                                                                                                                                                                                                                                                                                                                                random_number_in_between(100, 255)));
                                                                                                                                                                                                                                                                                                                                                                                                                                                                    }
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    }
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                }
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        }
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            }
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                return li_color;
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                }



Mat draw_segmentation(const Mat& mat_label, const Mat& im_bgr, const vector<Scalar>& li_bgr, int sb, int cb, float ratio_area, float ratio_length, int minV_int, int maxV_int)
{
    Mat im_bgr_segmented = im_bgr.clone();    
    //double minVal, maxVal; Point minLoc, maxLoc;
    //minMaxLoc(mat_label, &minVal, &maxVal, &minLoc, &maxLoc);
    Size sz = mat_label.size();
    int iL, iC = 0, th_area = ratio_area * sz.width * sz.height, th_width = ratio_length * sz.width, th_height = ratio_length * sz.height;//, minVal_int = minVal, maxVal_int = maxVal;
    //namedWindow("im_color_each", WINDOW_NORMAL);
    //vector<Scalar> li_bgr = generate_random_color_list(maxVal_int);
    for(iL = minV_int; iL <= maxV_int; iL++)
    {
        Mat im_color_each = Mat::zeros(sz, CV_8UC3), im_level_each;
        inRange(mat_label, iL, iL, im_level_each);
        int n_nz = countNonZero(im_level_each);
        if(n_nz < th_area) continue;
        vector<vector<Point> > contours;
        //vector<Vec4i> hierarchy;
        findContours(im_level_each, contours, /*hierarchy,*/ CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
        RotatedRect rect = minAreaRect(Mat(contours[0]));
        if(rect.size.width > th_width || rect.size.height > th_height) continue;
        drawContours(im_bgr_segmented, contours, 0, li_bgr[iC++], 2, 8);
    }
    float sz_font = 1.0;
    Scalar kolor = CV_RGB(0, 255, 0);
    int x_txt = MAX(10, im_bgr_segmented.cols * 0.5 - 100), y_txt = 40, y_int = 30, iT = 0;
    putText(im_bgr_segmented, "spatial : " + to_string(sb), Point(x_txt, y_txt + y_int * iT++), FONT_HERSHEY_DUPLEX, sz_font, kolor, 0.5, CV_AA); 
    putText(im_bgr_segmented, "color : " + to_string(cb), Point(x_txt, y_txt + y_int * iT++), FONT_HERSHEY_DUPLEX, sz_font, kolor, 0.5, CV_AA); 
    putText(im_bgr_segmented, "area : " + to_string(th_area), Point(x_txt, y_txt + y_int * iT++), FONT_HERSHEY_DUPLEX, sz_font, kolor, 0.5, CV_AA); 
    //imshow("im_color_each", im_bgr);   waitKey(1);
    //exit(0);
    return im_bgr_segmented; 
}

Size compute_size_smaller_than(const Size& size_src, const Size& size_tgt)
{
    Size size_smaller_than(size_src);
    if((size_src.width > size_tgt.width && size_src.height > size_tgt.height) ||
        (size_src.width < size_tgt.width && size_src.height < size_tgt.height))
    {
        bool shall_shrink = size_src.width > size_tgt.width;
        //cout << "shall shrink : " << shall_shrink << endl;  exit(0);
        for(int iS = 2; iS < 10000; iS++)
        {
            cout << "iS : " << iS << endl;
            if(shall_shrink)
            {
                //if(is_first_a_factor_of_second(iS, size_src.width) && is_first_a_factor_of_second(iS, size_src.height))
                //{
                    int wid = size_src.width / iS, hei = size_src.height / iS;
                    if(wid <= size_tgt.width || hei <= size_tgt.height)
                    {
                        size_smaller_than.width = wid;  size_smaller_than.height = hei;
                        break;
                    }
                //}
            }
            else
            {
                int wid = size_src.width * iS,  hei = size_src.height * iS;
                if(wid >= size_tgt.width && hei >= size_tgt.height)
                {
                    size_smaller_than.width = size_src.width * (iS - 1);
                    size_smaller_than.height = size_src.height * (iS - 1);
                    break;
                }
                else if(wid >= size_tgt.width || hei >= size_tgt.height)
                {
                    size_smaller_than.width = wid;  size_smaller_than.height = hei;
                    break;
                }
            }
        }
    }
    return size_smaller_than;
}


#include <algorithm>
bool is_only_number(const std::string& s)
{
    return !s.empty() && std::find_if(s.begin(), 
            s.end(), [](char c) { return !std::isdigit(c); }) == s.end();
            }


            //------------ Check if the string is camera index --------------  
            //  cout << is_this_camera_index("04");
            //  => true
            bool is_this_camera_index(const std::string& strin)
            {
                return 2 >= strin.size() && is_only_number(strin);  
                }



void on_trackbar(int, void*)
{
    //cout << "g_
    //printf("%d\n", g_slider);
}

bool proc_common(Output& output, const Input& input)
{
    Mat im_lab;
    //resize(input.m_im_bgr, im_bgr, sz_small);
	cvtColor(input.m_im_bgr, im_lab, CV_RGB2Lab);
    imshow("The Original Picture", input.m_im_bgr);
 
	MeanShift MSProc(input.m_spatial_bandwidth, input.m_color_bandwidth);
	Mat mat_label = MSProc.MSSegmentation(im_lab);
    //Mat mat_label = input.m_ms.MSSegmentation(im_lab);

    double minVal, maxVal; //Point minLoc, maxLoc;
    
    minMaxLoc(mat_label, &minVal, &maxVal);//, &minLoc, &maxLoc);
    
    int minV_int = minVal, maxV_int = maxVal;
    
    output.m_im_segmented = draw_segmentation(mat_label, input.m_im_bgr, input.m_li_bgr, input.m_spatial_bandwidth, input.m_color_bandwidth, input.m_ratio_area, input.m_ratio_length, minV_int, maxV_int);
    
    return true;
          
}

bool proc_img(const string& path_img, Input& input)
{
    Output output;
    input.m_im_bgr = cv::imread(path_img, CV_LOAD_IMAGE_COLOR);
    Size sz_small = compute_size_smaller_than(input.m_im_bgr.size(), Size(WIDTH_SMALL, HEIGHT_SMALL));  
    //Size sz_small = input.m_im_bgr.size();  
    resize(input.m_im_bgr, input.m_im_bgr, sz_small);
    //output.m_im_segmented = input.m_im_bgr.clone();
    input.m_li_bgr = generate_random_color_list(sz_small.width + sz_small.height);
    //cvtColor(input.im_bgr, input.im_gray, CV_BGR2GRAY);
    proc_common(output, input);
    imwrite("result/im_segmented.bmp", output.m_im_segmented);   
    namedWindow("im_segmented", WINDOW_NORMAL);    
    imshow("im_segmented", output.m_im_segmented); waitKey();   
    return true;
}


int proc_cam(int idx_cam, Input& input)
{
    Output output;
    Timer timer;    timer.Start();
    //비디오 캡쳐 초기화
    VideoCapture cap(idx_cam);
    if (!cap.isOpened()) {
        cerr << "에러 - 카메라를 열 수 없습니다.\n";    return false;
    }
    Size sz_cur(cap.get(CV_CAP_PROP_FRAME_WIDTH), cap.get(CV_CAP_PROP_FRAME_HEIGHT));
    Size sz_small = compute_size_smaller_than(sz_cur, Size(WIDTH_SMALL, HEIGHT_SMALL));  
    input.m_li_bgr = generate_random_color_list(sz_small.width + sz_small.height);
    //cap.set(CV_CAP_PROP_FRAME_WIDTH, 640);  cap.set(CV_CAP_PROP_FRAME_HEIGHT, 480);
    namedWindow("im_segmented", WINDOW_NORMAL);    
    while(1)
    {
        // 카메라로부터 캡쳐한 영상을 frame에 저장합니다.
        cap.read(input.m_im_bgr);
        if(input.m_im_bgr.empty()) {
            cerr << "빈 영상이 캡쳐되었습니다.\n";  return false;
        }
        resize(input.m_im_bgr, input.m_im_bgr, sz_small);
        // 영상을 화면에 보여줍니다.
        //output.m_im_segmented = input.m_im_bgr.clone();
        //cvtColor(input.im_bgr, input.im_gray, CV_BGR2GRAY);
        proc_common(output, input);
        imshow("im_segmented", output.m_im_segmented); 
        // ESC 키를 입력하면 루프가 종료됩니다.
        char ch = waitKey(1);
        if (27 == ch) break;
        else if('q' == ch || 'w' == ch || 'e' == ch ||  
                'a' == ch || 's' == ch || 'd' == ch ||  
                'z' == ch || 'x' == ch || 'c' == ch
                )
        {
            if('q' == ch) input.m_spatial_bandwidth = MAX(1, (1.0 - 0.2) * input.m_spatial_bandwidth);
            else if('e' == ch) input.m_spatial_bandwidth = MIN(100, (1.0 + 0.2) * input.m_spatial_bandwidth);
            else if('w' == ch) input.m_spatial_bandwidth = input.m_sb_init;

            else if('a' == ch) input.m_color_bandwidth = MAX(1, (1.0 - 0.2) * input.m_color_bandwidth);
            else if('d' == ch) input.m_color_bandwidth = MIN(100, (1.0 + 0.2) * input.m_color_bandwidth);
            else if('s' == ch) input.m_color_bandwidth = input.m_cb_init;
            
            else if('z' == ch) input.m_ratio_area = MAX(0.001, (1.0 - 0.3) * input.m_ratio_area);
            else if('x' == ch) input.m_ratio_area = MIN(0.5, (1.0 + 0.3) * input.m_ratio_area);
            else if('c' == ch) input.m_ratio_area = input.m_ra_init;
            
        }
        cout << "FPS : " << timer.updateFPS() << endl;
        //input.m_ms.reset(input.m_spatial_bandwidth, input.m_color_bandwidth);
    }
}


int main(int argc, char** argv)
{
    //if( argc !=2 )
    if( argc != 6 )
    {
        help(argv); return -1;
    }
    int bandwidth_spatial = atoi(argv[2]), bandwidth_color = atoi(argv[3]);
    float ratio_area = atof(argv[4]), ratio_length = atof(argv[5]);
    Input input(bandwidth_spatial, bandwidth_color, ratio_area, ratio_length);
    return is_this_camera_index(argv[1]) ? proc_cam(atoi(argv[1]), input) : proc_img(argv[1], input);
}    

#if 0
    VideoCapture cap;
    Mat im_lab, im_bgr;
    if(is_cam)
    {
        cap = VideoCapture(atoi(argv[1]));
        if (!cap.isOpened()) return -1;
    }
    else
    {
        im_bgr = imread( argv[1] );
    }         
    //g_spatial = atoi(argv[2]), g_color = atoi(argv[3]), g_area = atoi(argv[4]);
	// Load image
    //Mat im_lab, im_bgr = imread( argv[1] );
    Size sz_small = compute_size_smaller_than(im_bgr.size(), Size(320, 240));  
   
    cout << "sz_small : " << sz_small << endl;

    namedWindow("segmentation", WINDOW_NORMAL);    namedWindow("The Original Picture", WINDOW_NORMAL);
    createTrackbar("spatial", "segmentation", &bandwidth_spatial, g_spatial_max, on_trackbar);
    //setTrackbarPos("spaital bandwidth", "segmentation", bandwidth_spatial);
    createTrackbar("color", "segmentation", &bandwidth_color, g_color_max, on_trackbar);
    //setTrackbarPos("color bandwidth", "segmentation", bandwidth_color);
    createTrackbar("area", "segmentation", &th_area, g_area_max, on_trackbar);
//    setTrackbarPos("min area", "segmentation", th_area);

    vector<Scalar> li_bgr;// = generate_random_color_list(maxVal_int);
    if(!is_cam)
    {
        resize(im_bgr, im_bgr, sz_small);
	    cvtColor(im_bgr, im_lab, CV_RGB2Lab);
    }
    while(1)
    {
        bandwidth_spatial = getTrackbarPos("spatial", "segmentation");
        bandwidth_color = getTrackbarPos("color", "segmentation");
        th_area = getTrackbarPos("area", "segmentation");
        cout << "bandwidth_spatial : " << bandwidth_spatial << endl;
        cout << "bandwidth_color : " << bandwidth_color << endl;
        cout << "th_area : " << th_area << endl;
        if(is_cam)
        {
            cap >> im_bgr;
	        resize(im_bgr, im_bgr, sz_small);
	        cvtColor(im_bgr, im_lab, CV_RGB2Lab);
	        imshow("The Original Picture", im_bgr);
        }
        if(li_bgr.empty()) li_bgr = generate_random_color_list(sz_small.width + sz_small.height);
	    MeanShift MSProc(bandwidth_spatial, bandwidth_color);
	    Mat mat_label = MSProc.MSSegmentation(im_lab);
        double minVal, maxVal; //Point minLoc, maxLoc;
        minMaxLoc(mat_label, &minVal, &maxVal);//, &minLoc, &maxLoc);
        int minV_int = minVal, maxV_int = maxVal;
        Mat im_bgr_segmented = draw_segmentation(mat_label, im_bgr, li_bgr, th_area, minV_int, maxV_int);
        imshow("segmentation", im_bgr_segmented);   
	    imshow("The Original Picture", im_bgr);   
        int key = waitKey(1);   if(27 == key) break;
        
    }

    //cout << "b4 resize im_bgr.size() : " << im_bgr.size() << endl;
    //cout << "after resize im_bgr.size() : " << im_bgr.size() << endl;
	// Show that image

	// Convert color from RGB to Lab

	// Initilize Mean Shift with spatial bandwith and color bandwith
	//MeanShift MSProc(8, 16);
	//MeanShift MSProc(bandwidth_spatial, bandwidth_color);
	// Filtering Process
	//MSProc.MSFiltering(Img);
	// Segmentation Process include Filtering Process (Region Growing)
	//Mat mat_label = MSProc.MSSegmentation(im_lab);

    //display_each_color(mat_label, im_bgr, th_area);
/*
    double minVal, maxVal; Point minLoc, maxLoc;
    minMaxLoc(mat_label, &minVal, &maxVal, &minLoc, &maxLoc, Mat() );
    cout << "b4 minVal : " << minVal << ",\tmaxVal : " << maxVal << endl;
    Mat im_gray_label = mat_label * (255.0 / maxVal);   im_gray_label.convertTo(im_gray_label, CV_8UC1);
    minMaxLoc(im_gray_label, &minVal, &maxVal, &minLoc, &maxLoc, Mat() );
    cout << "im_gray_label.size() : " << im_gray_label.size() << endl;
    cout << "after minVal : " << minVal << ",\tmaxVal : " << maxVal << endl;
	namedWindow("im_gray_label", WINDOW_NORMAL);   imshow("im_gray_label", im_gray_label); //waitKey();  
    // Print the bandwith
	cout<<"the Spatial Bandwith is "<<MSProc.hs<<endl;
	cout<<"the Color Bandwith is "<<MSProc.hr<<endl;

	// Convert color from Lab to RGB
	cvtColor(im_lab, im_bgr, CV_Lab2RGB);
    cout << "after ms Img.size() : " << im_bgr.size() << endl;
	// Show the result image
	namedWindow("MS Picture", WINDOW_NORMAL);
	imshow("MS Picture", im_bgr);
*/
	//waitKey();
	return 1;
}

#endif
