#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>
#include <numeric>
#include <string>
#include <iostream>
using namespace std;

// Define a global client that can request services
ros::ServiceClient client;

static bool moving = false;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
    // TODO: Request a service and pass the velocities to it to drive the robot
   
    ball_chaser::DriveToTarget srv;
    srv.request.linear_x = lin_x;
    srv.request.angular_z = ang_z;


    if (!client.call(srv)) {
	    ROS_ERROR("Failed to call service DriveToTarget.");
	}
}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{

    int white_pixel = 255;
    // TODO: Loop through each pixel in the image and check if there's a bright white one
    // Then, identify if this pixel falls in the left, mid, or right side of the image
    // Depending on the white ball position, call the drive_bot function and pass velocities to it
    // Request a stop when there's no white ball seen by the camera

    int height = img.height;
    int width = img.width;
    int step = img.step;
    int left_pos = 0.5*img.width;
    int right_pos = 0.5*img.width;
    int centre_pos = 0;
    float x = 0.0;
    float z = 0.0;
  
    float offset_accumulated = 0;
    int count_total = 0;

    for (int i = 0; i < height ; i++) {
        for (int j = 0; j < step; j+=3) {
            if (img.data[i * step + j] == white_pixel && \
            img.data[i * step + j + 1] == white_pixel && \
            img.data[i * step + j + 2] == white_pixel) {
                if (left_pos > j){
                    left_pos = j;
                }
                else if (right_pos < j){
                    right_pos = j;
                }
                else
                {
                    nullptr;
                }
            }
        }
    }

    if (right_pos != left_pos){
        centre_pos = (right_pos - left_pos) * 0.5 + left_pos;

        if (centre_pos < img.step/9){
        // go left
        x = 0.2;
        z = 0.2;
        }
        else if (centre_pos > img.step*2/9){
        //GO RIGHT
        x = 0.2;
        z = -0.2;
        }
        else {
            //GO_STRAIGHT
        x = 0.2;
        z = 0.0;
        }
        
        drive_robot(x, z);
        moving = true;
    }
    else{
        if(moving) drive_robot(0.0, 0.0);
        moving = false;
    }
    
}



int main(int argc, char** argv)
{
    // Initialize the process_image node and create a handle to it
    ros::init(argc, argv, "process_image");
    ros::NodeHandle n;

    // Define a client service capable of requesting services from command_robot
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

    // Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
    ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

    // Handle ROS communication events
    ros::spin();

    return 0;
}

