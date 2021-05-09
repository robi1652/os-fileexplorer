#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <string>
#include <dirent.h>
#include <sys/stat.h>
#include <cstring>
#include <bits/stdc++.h>
#include <algorithm>
#include <vector>

#define WIDTH 800
#define HEIGHT 600

typedef struct AppData {
    std::vector<std::string> files;
    std::vector<std::string> file_sizes;
    TTF_Font *font;
    SDL_Texture *penguin;
    //SDL_Texture *phrase;
    std::vector<SDL_Texture*> files_textures;
    std::vector<SDL_Texture*> file_sizes_textures;
    SDL_Rect penguin_rect;
    std::vector<SDL_Rect*> files_rect;
} AppData;

void initialize(SDL_Renderer *renderer, AppData *data_ptr);
void render(SDL_Renderer *renderer, AppData *data_ptr);
void quit(AppData *data_ptr);

int main(int argc, char **argv)
{
    char *home = getenv("HOME");
    printf("HOME: %s\n", home);

    // initializing SDL as Video
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();

    // create window and renderer
    SDL_Renderer *renderer;
    SDL_Window *window;
    SDL_CreateWindowAndRenderer(WIDTH, HEIGHT, 0, &window, &renderer);

    // initialize and perform rendering loop
    AppData data;
    initialize(renderer, &data);
    render(renderer, &data);
    SDL_Event event;
    SDL_WaitEvent(&event);
    while (event.type != SDL_QUIT)
    {
        SDL_WaitEvent(&event);

        render(renderer, &data);
    }

    // clean up
    quit(&data);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();


    return 0;
}

void initialize(SDL_Renderer *renderer, AppData *data_ptr)
{
    data_ptr->font = TTF_OpenFont("resrc/OpenSans-Regular.ttf", 24);

    //SDL_Surface *img_surf = IMG_Load("resrc/images/tux.png");
    //data_ptr->penguin = SDL_CreateTextureFromSurface(renderer, img_surf);
    //SDL_FreeSurface(img_surf);
    //data_ptr->penguin_rect.x = 200;
    //data_ptr->penguin_rect.y = 100;
    //data_ptr->penguin_rect.w = 165;
    //data_ptr->penguin_rect.h = 200;

    struct stat info;
    int err = stat(std::string("/home").c_str(), &info);
    if (err == 0 && S_ISDIR(info.st_mode))
    {
        DIR* dir = opendir(std::string("/home").c_str());
        // TODO: modify to be able to print all entries in directory in alphabetical order
        //       in addition to file name, also print file size (or 'directory' if entry is a folder)
        //       Example output:
        //         ls.cpp (693 bytes)
        //         my_file.txt (62 bytes)
        //         OS Files (directory)
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            data_ptr->files.push_back(entry->d_name);
            //printf("%s ", entry->d_name);
            //printf("(%ld bytes)\n", info2.st_size);
        }
        closedir(dir);

        std::sort(data_ptr->files.begin(), data_ptr->files.end());

        int file_err;
        struct stat file_info;
        for (int i = 0; i < data_ptr->files.size(); i++) {
            SDL_Color color = { 0, 0, 0 };
            std::string full_path = "/home/" + data_ptr->files[i];
            file_err = stat(full_path.c_str(), &file_info);
            if (file_err) {
                fprintf(stderr, "Uh oh, we shouldn't get here\n");
            }
            else if (S_ISDIR(file_info.st_mode)) {
                printf("%s (directory)\n", data_ptr->files[i].c_str());
                data_ptr->file_sizes.push_back("(directory)");
                if (data_ptr->files[i] != ".") {
                }
            }
            else {
                printf("%s (%ld bytes)\n", data_ptr->files[i].c_str(), file_info.st_size);
                data_ptr->file_sizes.push_back(std::string((char*)file_info.st_size));
            }
            //file_err = stat(full_path.c_str(), &file_info);
            //std::string to_display = data_ptr->files[i] + std::string(" (") + std::string((char*)file_info.st_size) + std::string(")");
            SDL_Surface *phrase_surf = TTF_RenderText_Solid(data_ptr->font, data_ptr->files[i].c_str(), color);
            SDL_Texture *file = SDL_CreateTextureFromSurface(renderer, phrase_surf);
            data_ptr->files_textures.push_back(file);
            SDL_FreeSurface(phrase_surf);

            phrase_surf = TTF_RenderText_Solid(data_ptr->font, data_ptr->file_sizes[i].c_str(), color);
            SDL_Texture *file_size = SDL_CreateTextureFromSurface(renderer, phrase_surf);
            data_ptr->file_sizes_textures.push_back(file_size);
            SDL_FreeSurface(phrase_surf);
        }
    }

    //SDL_Color color = { 0, 0, 0 };
    //SDL_Surface *phrase_surf = TTF_RenderText_Solid(data_ptr->font, "Hello World!", color);
    //data_ptr->phrase = SDL_CreateTextureFromSurface(renderer, phrase_surf);
    //SDL_FreeSurface(phrase_surf);
}

void render(SDL_Renderer *renderer, AppData *data_ptr)
{
    // erase renderer content
    SDL_SetRenderDrawColor(renderer, 235, 235, 235, 255);
    SDL_RenderClear(renderer);
    
    // TODO: draw!
    //SDL_RenderCopy(renderer, data_ptr->penguin, NULL, &rect);
    int curr_x = 10;
    int curr_y = 10;

    for (int i = 0; i < data_ptr->files.size(); i++) {
        data_ptr->files_rect.push_back(new SDL_Rect);
        SDL_Rect rect = *data_ptr->files_rect[i];
        rect.x = curr_x;
        rect.y = curr_y;
        SDL_QueryTexture(data_ptr->files_textures[i], NULL, NULL, &(rect.w), &(rect.h));
        SDL_RenderCopy(renderer, data_ptr->files_textures[i], NULL, &rect);
        rect.x = rect.x + 200;
        SDL_QueryTexture(data_ptr->file_sizes_textures[i], NULL, NULL, &(rect.w), &(rect.h));
        SDL_RenderCopy(renderer, data_ptr->file_sizes_textures[i], NULL, &rect);
        curr_x;
        curr_y = curr_y + 30;
    }
    //SDL_QueryTexture(data_ptr->phrase, NULL, NULL, &(rect.w), &(rect.h));
    //rect.x = 10;
    //rect.y = 500;
    //SDL_RenderCopy(renderer, data_ptr->phrase, NULL, &rect);

    // show rendered frame
    SDL_RenderPresent(renderer);
}

void quit(AppData *data_ptr)
{
    SDL_DestroyTexture(data_ptr->penguin);

    //Destroy each file's texture
    int size = data_ptr->files_textures.size();
    for (int i = 0; i < size; i++) {
        SDL_DestroyTexture(data_ptr->files_textures[i]);
    }
    TTF_CloseFont(data_ptr->font);
}
