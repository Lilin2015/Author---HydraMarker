#include <iostream>
#include "generator_HydraMarker.h"

void example_A();   // 12x12 3-order marker field by fast-bWFC
void example_B();   // 50x50 4-order marker field by the genetic algorithm (larger size is possible, such as 75x75, but it might cost days/weeks)

void example_Ad1();   // 85x85 4-order marker field by fast-bWFC
void example_Ad2();   // 90x90 circular marker field, 4x4 tag shape
void example_Ad3();   // 90x90 marker field, 4x4 tag shape, four QR locators
void example_Ad4();   // 123x123 marker field, 5x5 tag shape, build-in locator array
void example_Ad5();   // 60x60 marker field, 4x4, 3x6, 2x9, 1x20 tag shapes
void example_Ad6();   // 85x85 marker field, frame shape, 4x4, 3x6, 2x9, 1x20 tag shapes

void main()
{
    // uncomment the example you would like to check
    
    // example_A();
    // example_B();
    
    // example_Ad1();
    // example_Ad2();
    // example_Ad3();
    // example_Ad4();
    // example_Ad5();
    // example_Ad6();

    return;
}

void example_A()
{
    generator_HydraMarker gen;

    // set time/step limit, if necessary 
    double max_ms = 600000;
    int max_trial = INT_MAX;

    // set log path
    string log_path = "generate.log";

    // build initial marker field
    Mat1b state = 2 * Mat1b::ones(12, 12);

    // set tag shape
    Mat1b shape1 = Mat1b::ones(3, 3);

    // generate (use release mode for speed)
    gen.set_field(state);
    gen.set_tagShape(vector<Mat1b>{shape1});
    gen.generate(METHOD::FBWFC, max_ms, max_trial, false, log_path); // set show_process = false for speed

    // save
    gen.save("MF.field");

    // show result
    gen.show();
    waitKey(0);
}
void example_B()
{
    generator_HydraMarker gen;

    // set time/step limit, if necessary 
    double max_ms = 600000;
    int max_trial = INT_MAX;

    // set log path
    string log_path = "generate.log";

    // build initial marker field
    Mat1b state = 2 * Mat1b::ones(50, 50);

    // set tag shape
    Mat1b shape1 = Mat1b::ones(4, 4);

    // generate (use release mode for speed)
    gen.set_field(state);
    gen.set_tagShape(vector<Mat1b>{shape1});
    gen.generate(METHOD::GENETIC, max_ms, max_trial, false, log_path); // set show_process = false for speed

    // save
    gen.save("MF.field");

    // show result
    gen.show();
    waitKey(0);
}

void example_Ad1()
{
    generator_HydraMarker gen;

    // set time/step limit, if necessary 
    double max_ms = 600000;
    int max_trial = INT_MAX;

    // set log path
    string log_path = "generate.log";

    // build initial marker field
    Mat1b state = 2 * Mat1b::ones(85, 85);

    // set tag shape
    Mat1b shape1 = Mat1b::ones(4, 4);

    // generate (use release mode for speed)
    gen.set_field(state);
    gen.set_tagShape(vector<Mat1b>{shape1});
    gen.generate(METHOD::FBWFC, max_ms, max_trial, false, log_path); // set show_process = false for speed

    // save
    gen.save("MF.field");

    // show result
    Mat3b stateShow = gen.show();
    imwrite("Ad1.png", stateShow);
    waitKey(0);
}
void example_Ad2()
{
    generator_HydraMarker gen;

    // set time/step limit, if necessary 
    double max_ms = 600000;
    int max_trial = INT_MAX;

    // set log path
    string log_path = "generate.log";

    // build initial marker field
    Mat1b state = 2 * Mat1b::ones(90, 90);
    for (size_t im = 0; im < state.rows; im++)
    {
        for (size_t in = 0; in < state.cols; in++)
        {
            double r = sqrt(pow(im - 44.5, 2) + pow(in - 44.5, 2));
            if (r > 45)   state(im, in) = 3;
        }
    }
    // set tag shape
    Mat1b shape1 = Mat1b::ones(4, 4);

    // generate (use release mode for speed)
    gen.set_field(state);
    gen.set_tagShape(vector<Mat1b>{shape1});
    gen.generate(METHOD::FBWFC, max_ms, max_trial, false, log_path); // set show_process = false for speed

    // save
    gen.save("MF.field");

    // show result
    Mat3b stateShow = gen.show();
    imwrite("Ad2.png", stateShow);
    waitKey(0);
}
void example_Ad3()
{
    generator_HydraMarker gen;

    // set time/step limit, if necessary 
    double max_ms = 3600000;
    int max_trial = INT_MAX;

    // set log path
    string log_path = "generate.log";

    // build initial marker field
    Mat1b state = 2 * Mat1b::ones(90, 90);
    state.rowRange(0, 22).colRange(0, 22) = 3;
    state.rowRange(0, 22).colRange(68, 90) = 3;
    state.rowRange(68, 90).colRange(0, 20) = 3;
    state.rowRange(68, 90).colRange(68, 90) = 3;

    // set tag shape
    Mat1b shape1 = Mat1b::ones(4, 4);

    // generate (use release mode for speed)
    gen.set_field(state);
    gen.set_tagShape(vector<Mat1b>{shape1});
    gen.generate(METHOD::FBWFC, max_ms, max_trial, 0, log_path); // set show_process = false for speed

    // save
    gen.save("MF.field");

    // insert locators, show result
    Mat3b stateShow = gen.show();
    Mat3b locator(22, 22, Vec3b(255, 255, 255));

    locator.rowRange(7, 15).colRange(7, 15) = Scalar(0, 0, 255);
    locator.rowRange(1, 4).colRange(1, 21) = Scalar(0, 0, 255);
    locator.rowRange(18, 21).colRange(1, 21) = Scalar(0, 0, 255);
    locator.colRange(1, 4).rowRange(1, 21) = Scalar(0, 0, 255);
    locator.colRange(18, 21).rowRange(1, 21) = Scalar(0, 0, 255);

    locator.copyTo(stateShow.rowRange(0, 22).colRange(0, 22));
    locator.copyTo(stateShow.rowRange(0, 22).colRange(68, 90));
    locator.copyTo(stateShow.rowRange(68, 90).colRange(0, 22));
    locator.copyTo(stateShow.rowRange(68, 90).colRange(68, 90));

    namedWindow("state", WINDOW_NORMAL);
    imshow("state", stateShow);
    imwrite("Ad3.png", stateShow);
    waitKey(0);
}
void example_Ad4()
{
    generator_HydraMarker gen;

    // set time/step limit, if necessary 
    double max_ms = 3600000;
    int max_trial = INT_MAX;

    // set log path
    string log_path = "generate.log";

    // build initial marker field
    Mat1b state = 2 * Mat1b::ones(123, 123);
    for (size_t im = 1; im < state.rows; im+=12)
        for (size_t in = 1; in < state.cols; in+=12)
            state(im, in) = 0;
    Mat set_one;
    dilate(state == 0, set_one, getStructuringElement(0, Size(3, 3)));
    set_one.setTo(0, state == 0);
    state.setTo(1, set_one);

    // set tag shape
    Mat1b shape1 = Mat1b::ones(5, 5);

    // generate (use release mode for speed)
    gen.set_field(state);
    gen.set_tagShape(vector<Mat1b>{shape1});
    gen.generate(METHOD::FBWFC, max_ms, max_trial, false, log_path); // set show_process = false for speed

    // save
    gen.save("MF.field");

    // insert locators, show result
    Mat3b stateShow = gen.show();
    stateShow.setTo(Scalar(0, 255, 0), state == 1);
    stateShow.setTo(Scalar(0, 0, 255), state == 0);

    namedWindow("state", WINDOW_NORMAL);
    imshow("state", stateShow);
    imwrite("Ad4.png", stateShow);
    waitKey(0);
}
void example_Ad5()
{
    generator_HydraMarker gen;

    // set time/step limit, if necessary 
    double max_ms = 3600000;
    int max_trial = INT_MAX;

    // set log path
    string log_path = "generate.log";

    // build initial marker field
    Mat1b state = 2 * Mat1b::ones(60, 60);

    // set tag shape
    Mat1b shape1 = Mat1b::ones(4, 4);
    Mat1b shape2 = Mat1b::ones(3, 6);
    Mat1b shape3 = Mat1b::ones(2, 9);
    Mat1b shape4 = Mat1b::ones(1, 20);

    // generate (use release mode for speed)
    gen.set_field(state);
    gen.set_tagShape(vector<Mat1b>{shape1, shape2, shape3, shape4});
    gen.generate(METHOD::FBWFC, max_ms, max_trial, false, log_path); // set show_process = false for speed

    // save
    gen.save("MF.field");

    // show result
    Mat3b stateShow = gen.show();
    imwrite("Ad5.png", stateShow);
    waitKey(0);
}
void example_Ad6()
{
    generator_HydraMarker gen;

    // set time/step limit, if necessary 
    double max_ms = 600000;
    int max_trial = INT_MAX;

    // set log path
    string log_path = "generate.log";

    // build initial marker field
    Mat1b state = 3 * Mat1b::ones(85, 85);
    state.rowRange(0, 9) = 2;
    state.rowRange(76, 85) = 2;
    state.colRange(0, 9) = 2;
    state.colRange(76, 85) = 2;

    // set tag shape
    Mat1b shape1 = Mat1b::ones(4, 4);
    Mat1b shape2 = Mat1b::ones(3, 6);
    Mat1b shape3 = Mat1b::ones(2, 9);
    Mat1b shape4 = Mat1b::ones(1, 20);

    // generate (use release mode for speed)
    gen.set_field(state);
    gen.set_tagShape(vector<Mat1b>{shape1, shape2, shape3, shape4});
    gen.generate(METHOD::FBWFC, max_ms, max_trial, true, log_path); // set show_process = false for speed

    // save
    gen.save("MF.field");

    // show result
    Mat3b stateShow = gen.show();
    imwrite("Ad6.png", stateShow);
    waitKey(0);
}