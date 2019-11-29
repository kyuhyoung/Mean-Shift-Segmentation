//--------------------------------------------------------------------
//				Bingyang Liu  2015/07/14
//				Meanshift Algorithm
//				OpenCV 3.0.0
//--------------------------------------------------------------------

//---------------- Head  File ---------------------------------------
#include <iostream>
#include "MeanShift.h"

static void help(char** argv)
{
    cout << "\nDemonstrate mean-shift based color segmentation in spatial pyramid.\n"
    << "Call:\n   " << argv[0] << " image spatial_bandwidth color_bandwidth th_area\n"
    << "This program allows you to set the spatial and color radius\n"
    << "of the mean shift window as well as the number of pyramid reduction levels explored\n"
    << endl;
}


void display_each_color(const Mat& mat_label, const Mat& im_bgr, int th_area)
{
    double minVal, maxVal; Point minLoc, maxLoc;
    minMaxLoc(mat_label, &minVal, &maxVal, &minLoc, &maxLoc);
    for(int iL = int(minVal); iL <= int(maxVal); iL++)
    {
        Mat im_color_each = Mat::zeros(mat_label.size(), CV_8UC3), im_level_each;
        inRange(mat_label, iL, iL, im_level_each);
        int n_nz = countNonZero(im_level_each);
        //cout << "iL : " << iL << ",\t n_nz : " << n_nz << endl;
        if(n_nz < th_area) continue;
        im_bgr.copyTo(im_color_each, im_level_each);
        imshow("im_color_each", im_color_each); waitKey();

    }
    exit(0);
 
}

Size compute_size_smaller_than(const Size& size_src, const Size& size_tgt)
{
    cout << "size_src : " << size_src << endl << "size_tgt : " << size_tgt << endl;
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





int main(int argc, char** argv){

    //if( argc !=2 )
    if( argc != 5 )
    {
        help(argv);
        return -1;
    }

	// Load image
	//Mat Img = imread("mandril_color.tif");
    Mat im_lab, im_bgr = imread( argv[1] );
    int bandwidth_spatial = atoi(argv[2]), bandwidth_color = atoi(argv[3]), th_area = atoi(argv[4]);
    Size sz_small = compute_size_smaller_than(im_bgr.size(), Size(320, 240));  
	resize(im_bgr, im_bgr, sz_small);
	// Show that image
	namedWindow("The Original Picture");
	imshow("The Original Picture", im_bgr);

	// Convert color from RGB to Lab
	cvtColor(im_bgr, im_lab, CV_RGB2Lab);

	// Initilize Mean Shift with spatial bandwith and color bandwith
	//MeanShift MSProc(8, 16);
	MeanShift MSProc(bandwidth_spatial, bandwidth_color);
	// Filtering Process
	//MSProc.MSFiltering(Img);
	// Segmentation Process include Filtering Process (Region Growing)
	Mat mat_label = MSProc.MSSegmentation(im_lab);

    display_each_color(mat_label, im_bgr, th_area);

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

	waitKey();
	return 1;
}
