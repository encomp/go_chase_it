#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>

// Define a global client that can request services
ros::ServiceClient client; // This function calls the command_robot service to drive the robot in the specified direction

// Defines the direction to drive
enum class Direction
{
    Forward,
    Left,
    Right,
    Stop
};

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
    ROS_INFO_STREAM("Driving the robot");
    ball_chaser::DriveToTarget service;
    service.request.linear_x = lin_x;
    service.request.angular_z = ang_z;
    if (!client.call(service))
    {
        ROS_ERROR("Failed to call command_robot");
    }
}

void drive_robot(Direction direction)
{
    if (Direction::Left == direction)
    {
        drive_robot(0.0, 0.5);
    }
    else if (Direction::Forward == direction)
    {
        drive_robot(0.5, 0.0);
    }
    else if (Direction::Right == direction)
    {
        drive_robot(0.0, -0.5);
    }
    else if (Direction::Stop == direction)
    {
        drive_robot(0.0, 0.0);
    }
}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{
    Direction direction = Direction::Stop;
    int all_pixels_are_white = 765;
    bool white_seen = false;
    int one_third = floor(img.width / 3);
    for (int i = 0; i < img.height * img.width; i++)
    {
        int sum = img.data[i * 3] + img.data[i * 3 + 1] + img.data[i * 3 + 2];
        if (sum == all_pixels_are_white)
        {
            int xpos = i % img.width;
            if (xpos < one_third)
            {
                direction = Direction::Left;
            }
            else if (xpos >= one_third && xpos <= one_third * 2)
            {
                direction = Direction::Forward;
            }
            else
            {
                direction = Direction::Right;
            }
            break;
        }
    }

    drive_robot(direction);
}

int main(int argc, char **argv)
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
