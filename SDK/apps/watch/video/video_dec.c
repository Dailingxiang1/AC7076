#include "avilib.h"

//读取AVI文件视频帧和音频帧行写卡在PC上验证
int video_dec_demo(void)
{
    char *filename = "storage/sd0/C/VID_0001.AVI";
    avi_t *in_fd = AVI_open_input_file(filename, 1);
    if (in_fd == NULL) {
        printf("AVI_open_input_file err \n");
        return -1;
    }

    u8 *file_buf = malloc(20 * 1024);
    if (!file_buf) {
        printf("file_buf malloc err\n");
        return -1;
    }

    int total_frame = AVI_video_frames(in_fd);
    printf("AVI total video frame:%d \n", total_frame);
    int audio_chunks =  AVI_audio_chunks(in_fd);
    printf("AVI total audio frame:%d \n", audio_chunks);

    char save_file_name[64];
    int i = 0;
    for (i = 0; i < total_frame; i++) {
        AVI_set_video_position(in_fd, i);

        int keyframe;
        int file_len = AVI_read_frame(in_fd, (char *)file_buf, &keyframe);
        if (file_len > 0) {
            printf("write video frame:%d \n", i);
            snprintf(save_file_name, sizeof(save_file_name), "storage/sd0/C/avi_test/jpg_%d.jpg", i);
            FILE *fd = fopen(save_file_name, "w+");
            fwrite(file_buf, 1, file_len, fd);
            fclose(fd);
        }
    }

    snprintf(save_file_name, sizeof(save_file_name), "storage/sd0/C/avi_test/test.pcm");
    FILE *fd = fopen(save_file_name, "w+");

    u32 offset = 0;
    for (i = 0; i < audio_chunks; i++) {
        int audio_chunk_size =  AVI_audio_size(in_fd, i);
        AVI_set_audio_position(in_fd, offset);


        int file_len = AVI_read_audio_chunk(in_fd, (char *)file_buf);
        if (file_len > 0) {
            printf("write audio frame:%d  size:%d chunk_size:%d \n", i, file_len, audio_chunk_size);
            fwrite(file_buf, 1, file_len, fd);
        }

        offset += audio_chunk_size;
    }
    fclose(fd);


    AVI_close(in_fd);
    free(file_buf);

    /* os_time_dly(-1); */
    return 0;
}
