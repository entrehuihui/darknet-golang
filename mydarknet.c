#include "darknet.h"

void mydraw_detections(image im, detection *dets, int num, float thresh, char **names, image **alphabet, int classes);
// -------------------------------//
char **retNames()
{
    list *options = read_data_cfg("cfg/coco.data");
    char *name_list = option_find_str(options, "names", "data/names.list");
    // char **names = get_labels(name_list);
    return get_labels(name_list);
}

network *retNet()
{
    network *net = load_network("cfg/yolov3.cfg", "yolov3.weights", 0);
    set_batch_network(net, 1);
    srand(2222222);
    return net;
}

void testDetector(char *filename, float thresh, float hier_thresh, char *outfile, int fullscreen, char **names, image **alphabet, network *net)
{
    // char *filename = "data/dog.jpg";
    // float thresh = 0.5;
    // float hier_thresh = 0.5;
    // char *outfile = 0;
    // int fullscreen = 0;
    // char **names = retNames();
    // image **alphabet = load_alphabet();

    // 可分离
    // network *net = retNet();
    double time;
    char buff[256];
    char *input = buff;
    float nms = .45;
    while (1)
    {
        if (filename)
        {
            strncpy(input, filename, 256);
        }
        else
        {
            printf("Enter Image Path: ");
            fflush(stdout);
            input = fgets(input, 256, stdin);
            if (!input)
                return;
            strtok(input, "\n");
        }
        image im = load_image_color(input, 0, 0);
        image sized = letterbox_image(im, net->w, net->h);
        //image sized = resize_image(im, net->w, net->h);
        //image sized2 = resize_max(im, net->w);
        //image sized = crop_image(sized2, -((net->w - sized2.w)/2), -((net->h - sized2.h)/2), net->w, net->h);
        //resize_network(net, sized.w, sized.h);
        layer l = net->layers[net->n - 1];

        float *X = sized.data;
        time = what_time_is_it_now();
        network_predict(net, X);
        printf("%s: Predicted in %f seconds.\n", input, what_time_is_it_now() - time);
        int nboxes = 0;
        detection *dets = get_network_boxes(net, im.w, im.h, thresh, hier_thresh, 0, 1, &nboxes);
        //printf("%d\n", nboxes);
        //if (nms) do_nms_obj(boxes, probs, l.w*l.h*l.n, l.classes, nms);
        if (nms)
            do_nms_sort(dets, nboxes, l.classes, nms);
        mydraw_detections(im, dets, nboxes, thresh, names, alphabet, l.classes);
        free_detections(dets, nboxes);
        if (outfile)
        {
            save_image(im, outfile);
        }
        else
        {
            save_image(im, "predictions");
#ifdef OPENCV
            make_window("predictions", 512, 512, 0);
            show_image(im, "predictions", 0);
#endif
        }

        free_image(im);
        free_image(sized);
        if (filename)
            break;
    }
}

void mydraw_detections(image im, detection *dets, int num, float thresh, char **names, image **alphabet, int classes)
{
    int i, j;

    for (i = 0; i < num; ++i)
    {
        char labelstr[4096] = {0};
        int class = -1;
        for (j = 0; j < classes; ++j)
        {
            if (dets[i].prob[j] > thresh)
            {
                if (class < 0)
                {
                    strcat(labelstr, names[j]);
                    class = j;
                }
                else
                {
                    strcat(labelstr, ", ");
                    strcat(labelstr, names[j]);
                }
                printf("%s: %.0f%%\n", names[j], dets[i].prob[j] * 100);
            }
        }
        if (class >= 0)
        {
            int width = im.h * .006;

            /*
               if(0){
               width = pow(prob, 1./2.)*10+1;
               alphabet = 0;
               }
             */

            //printf("%d %s: %.0f%%\n", i, names[class], prob*100);
            int offset = class * 123457 % classes;
            float red = get_color(2, offset, classes);
            float green = get_color(1, offset, classes);
            float blue = get_color(0, offset, classes);
            float rgb[3];

            //width = prob*20+2;

            rgb[0] = red;
            rgb[1] = green;
            rgb[2] = blue;
            box b = dets[i].bbox;
            //printf("%f %f %f %f\n", b.x, b.y, b.w, b.h);

            int left = (b.x - b.w / 2.) * im.w;
            int right = (b.x + b.w / 2.) * im.w;
            int top = (b.y - b.h / 2.) * im.h;
            int bot = (b.y + b.h / 2.) * im.h;

            if (left < 0)
                left = 0;
            if (right > im.w - 1)
                right = im.w - 1;
            if (top < 0)
                top = 0;
            if (bot > im.h - 1)
                bot = im.h - 1;

            draw_box_width(im, left, top, right, bot, width, red, green, blue);
            if (alphabet)
            {
                image label = get_label(alphabet, labelstr, (im.h * .03));
                draw_label(im, top + width, left, label, rgb);
                free_image(label);
            }
            if (dets[i].mask)
            {
                image mask = float_to_image(14, 14, 1, dets[i].mask);
                image resized_mask = resize_image(mask, b.w * im.w, b.h * im.h);
                image tmask = threshold_image(resized_mask, .5);
                embed_image(tmask, im, left, top);
                free_image(mask);
                free_image(resized_mask);
                free_image(tmask);
            }
        }
    }
}

void test_detector()
{
    char *filename = "data/dog.jpg";
    float thresh = 0.5;
    float hier_thresh = 0.5;
    char *outfile = 0;
    int fullscreen = 0;
    char **names = retNames();
    image **alphabet = load_alphabet();

    // 可分离
    network *net = retNet();

    double time;
    char buff[256];
    char *input = buff;
    float nms = .45;
    while (1)
    {
        if (filename)
        {
            strncpy(input, filename, 256);
        }
        else
        {
            printf("Enter Image Path: ");
            fflush(stdout);
            input = fgets(input, 256, stdin);
            if (!input)
                return;
            strtok(input, "\n");
        }
        image im = load_image_color(input, 0, 0);
        image sized = letterbox_image(im, net->w, net->h);
        //image sized = resize_image(im, net->w, net->h);
        //image sized2 = resize_max(im, net->w);
        //image sized = crop_image(sized2, -((net->w - sized2.w)/2), -((net->h - sized2.h)/2), net->w, net->h);
        //resize_network(net, sized.w, sized.h);
        layer l = net->layers[net->n - 1];

        float *X = sized.data;
        time = what_time_is_it_now();
        network_predict(net, X);
        printf("%s: Predicted in %f seconds.\n", input, what_time_is_it_now() - time);
        int nboxes = 0;
        detection *dets = get_network_boxes(net, im.w, im.h, thresh, hier_thresh, 0, 1, &nboxes);
        //printf("%d\n", nboxes);
        //if (nms) do_nms_obj(boxes, probs, l.w*l.h*l.n, l.classes, nms);
        if (nms)
            do_nms_sort(dets, nboxes, l.classes, nms);
        draw_detections(im, dets, nboxes, thresh, names, alphabet, l.classes);
        free_detections(dets, nboxes);
        if (outfile)
        {
            save_image(im, outfile);
        }
        else
        {
            save_image(im, "predictions");
#ifdef OPENCV
            make_window("predictions", 512, 512, 0);
            show_image(im, "predictions", 0);
#endif
        }

        free_image(im);
        free_image(sized);
        if (filename)
            break;
    }
}

int checkFormat(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "usage: %s <function>\n", argv[0]);
        exit(0);
    }
    // gpu_index = find_int_arg(argc, argv, "-i", 0);
    // if (find_arg(argc, argv, "-nogpu"))
    // {
    //     gpu_index = -1;
    // }
    float thresh = find_float_arg(argc, argv, "-thresh", .5);
    char *filename = (argc > 4) ? argv[4] : 0;
    char *outfile = find_char_arg(argc, argv, "-out", 0);
    int fullscreen = find_arg(argc, argv, "-fullscreen");
    printf("%s, %s, %s, %f, %s, %d\n", argv[2], argv[3], filename, thresh, outfile, fullscreen);
    test_detector();
    return 1;
}
