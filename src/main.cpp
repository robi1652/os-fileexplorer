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
#include <filesystem>
#include <fstream>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <iterator>

#define WIDTH 800
#define HEIGHT 600

typedef struct AppData {
    std::vector<std::string> files;
    std::vector<std::string> files_displayed;
    std::vector<int> file_sizes;
    TTF_Font *font;
    SDL_Texture *directory_icon;
    SDL_Texture *image_icon;
    SDL_Texture *right_arrow_icon;
    SDL_Texture *left_arrow_icon;
    SDL_Texture *file_icon;
    SDL_Texture *code_icon;
    SDL_Texture *executable_icon;
    SDL_Texture *video_icon;
    //SDL_Texture *phrase;
    std::vector<SDL_Texture*> files_textures;
    std::vector<SDL_Texture*> file_sizes_textures;
    std::vector<SDL_Texture*> permissions_textures;
    std::vector<SDL_Rect*> files_rect;
    std::vector<SDL_Rect*> sizes_rect;
    std::vector<SDL_Rect*> icon_rect;
    std::vector<SDL_Rect*> permissions_rect;
    SDL_Rect right_arrow_rect;
    SDL_Rect left_arrow_rect;
    int page = 0;
    std::string current_directory;
    std::string previous_directory;
} AppData;

void initialize(SDL_Renderer *renderer, AppData *data_ptr);
void render(SDL_Renderer *renderer, AppData *data_ptr);
void quit(AppData *data_ptr);
std::string clickedCheck(AppData *data_ptr, SDL_Event *event);
void clickedOnDirectory(AppData *data_ptr);
void updateFileList(SDL_Renderer *renderer, AppData *data_ptr);

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
    clickedOnDirectory(&data);
    render(renderer, &data);
    SDL_Event event;
    SDL_WaitEvent(&event);
    while (event.type != SDL_QUIT)
    {
        SDL_WaitEvent(&event);
        switch (event.type)
        {
            case SDL_MOUSEBUTTONDOWN:
                std::string itemClicked = clickedCheck(&data, &event);
                //if (itemClicked == "RIGHT ARROW" || itemClicked == "LEFT ARROW") {
                //    data.files.erase(data.files.begin(), data.files.end());
                //    data.files = data.files_displayed;
                //    continue;
                //}
                
                struct stat fileStat;
                //Need filepath
                if (stat((data.current_directory + "/" + itemClicked).c_str(), &fileStat) == 0) {
                    if (fileStat.st_mode & S_IFDIR) {
                        data.previous_directory = data.current_directory;
                        if (itemClicked != "..") {
                            data.current_directory += "/" + itemClicked;
                        }
                        data.page = 0;
                        printf("%s\n", data.current_directory.c_str());
                        updateFileList(renderer, &data);
                        clickedOnDirectory(&data);
                    } else if (fileStat.st_mode & S_IFREG){
                        //It's a file, call a method to run it
                        int pid = fork();
                        if (pid == 0) {
                            execl("/usr/bin/xdg-open", "xdg-open", (data.current_directory + "/" + itemClicked).c_str(), (char *)0);
                            exit(1);
                        }
                    }
                } 
                clickedOnDirectory(&data);
                //How do we know what's a directory and what's a file
                printf("%s\n", itemClicked.c_str());
                for (int i = 0; i < data.files_displayed.size(); i++) {
                    printf("%s\n", data.files_displayed[i].c_str());
                }
        }
        
        
        //Should be an if check to make sure it's actually a directory
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
    data_ptr->font = TTF_OpenFont("resrc/OpenSans-Regular.ttf", 20);

    struct stat info;
    std::cout << getenv("HOME") << std::endl;
    data_ptr->current_directory = std::string(getenv("HOME"));
    data_ptr->previous_directory = data_ptr->current_directory;
    int err = stat(getenv("HOME"), &info);
    if (err == 0 && S_ISDIR(info.st_mode))
    {
        DIR* dir = opendir(getenv("HOME"));
        // TODO: modify to be able to print all entries in directory in alphabetical order
        //       in addition to file name, also print file size (or 'directory' if entry is a folder)
        //       Example output:
        //         ls.cpp (693 bytes)
        //         my_file.txt (62 bytes)
        //         OS Files (directory)
        struct dirent *entry;
        int q = 0;
        while ((entry = readdir(dir)) != NULL) {
            data_ptr->files.push_back(entry->d_name);
            if (q > data_ptr->page*10 && q < data_ptr->page*10+10) {
                data_ptr->files_displayed.push_back(entry->d_name);
            }
            q++;
        }
        closedir(dir);

        std::sort(data_ptr->files.begin(), data_ptr->files.end());
        data_ptr->files.erase(data_ptr->files.begin());

        int file_err;
        struct stat file_info;
        for (int i = 0; i < data_ptr->files.size(); i++) {
            SDL_Color color = { 0, 0, 0 };
            std::string full_path = getenv("HOME") + std::string("/") + data_ptr->files[i];
            file_err = stat(full_path.c_str(), &file_info);
            if (file_err) {
                fprintf(stderr, "Uh oh, we shouldn't get here\n");
            }
            else if (S_ISDIR(file_info.st_mode)) {
                //printf("%s (directory)\n", data_ptr->files[i].c_str());
                data_ptr->file_sizes.push_back(-1);
            }
            else {
                //printf("%s (%ld bytes)\n", data_ptr->files[i].c_str(), file_info.st_size);
                data_ptr->file_sizes.push_back(file_info.st_size);
                std::string file_permissions = std::string(( (file_info.st_mode & S_IRUSR) ? "r" : "-")) + std::string(( (file_info.st_mode & S_IWUSR) ? "w" : "-"))
                + std::string(( (file_info.st_mode & S_IXUSR) ? "x" : "-")) + std::string(( (file_info.st_mode & S_IRGRP) ? "r" : "-")) +
                std::string(( (file_info.st_mode & S_IWGRP) ? "w" : "-")) + std::string(( (file_info.st_mode & S_IXGRP) ? "x" : "-")) +
                std::string(( (file_info.st_mode & S_IROTH) ? "r" : "-")) + std::string(( (file_info.st_mode & S_IWOTH) ? "w" : "-")) +
                std::string(( (file_info.st_mode & S_IXOTH) ? "x" : "-"));
                //SDL_Surface *phrase_surf = TTF_RenderText_Solid(data_ptr->font, file_permissions.c_str(), color);
                //SDL_Texture *file_permissions_texture = SDL_CreateTextureFromSurface(renderer, phrase_surf);
                //data_ptr->permissions_textures.push_back(file_permissions_texture);
                //SDL_FreeSurface(phrase_surf);
            }
            SDL_Surface *phrase_surf = TTF_RenderText_Solid(data_ptr->font, data_ptr->files[i].c_str(), color);
            SDL_Texture *file = SDL_CreateTextureFromSurface(renderer, phrase_surf);
            data_ptr->files_textures.push_back(file);
            SDL_FreeSurface(phrase_surf);

            std::string suffix = std::string(" B");
            if (data_ptr->file_sizes[i] >= 1024) {
                suffix = std::string(" KB");
                data_ptr->file_sizes[i] = data_ptr->file_sizes[i] / 1024;
            }
            if (data_ptr->file_sizes[i] >= 1024) {
                suffix = std::string(" KB");
                data_ptr->file_sizes[i] = data_ptr->file_sizes[i] / 1024;
            }
            if (data_ptr->file_sizes[i] >= 1024) {
                suffix = std::string(" MB");
                data_ptr->file_sizes[i] = data_ptr->file_sizes[i] / 1024;
            }
            if (data_ptr->file_sizes[i] >= 1024) {
                suffix = std::string(" GB");
                data_ptr->file_sizes[i] = data_ptr->file_sizes[i] / 1024;
            }
            std::string file_size1 = std::to_string(data_ptr->file_sizes[i]) + suffix;
            phrase_surf = TTF_RenderText_Solid(data_ptr->font, file_size1.c_str(), color);
            SDL_Texture *file_size = SDL_CreateTextureFromSurface(renderer, phrase_surf);
            data_ptr->file_sizes_textures.push_back(file_size);
            SDL_FreeSurface(phrase_surf);

            std::string file_permissions = std::string(( (file_info.st_mode & S_IRUSR) ? "r" : "-")) + std::string(( (file_info.st_mode & S_IWUSR) ? "w" : "-"))
                + std::string(( (file_info.st_mode & S_IXUSR) ? "x" : "-")) + std::string(( (file_info.st_mode & S_IRGRP) ? "r" : "-")) +
                std::string(( (file_info.st_mode & S_IWGRP) ? "w" : "-")) + std::string(( (file_info.st_mode & S_IXGRP) ? "x" : "-")) +
                std::string(( (file_info.st_mode & S_IROTH) ? "r" : "-")) + std::string(( (file_info.st_mode & S_IWOTH) ? "w" : "-")) +
                std::string(( (file_info.st_mode & S_IXOTH) ? "x" : "-"));
            phrase_surf = TTF_RenderText_Solid(data_ptr->font, file_permissions.c_str(), color);
            SDL_Texture *file_permissions_texture = SDL_CreateTextureFromSurface(renderer, phrase_surf);
            data_ptr->permissions_textures.push_back(file_permissions_texture);
            SDL_FreeSurface(phrase_surf);
        }

        SDL_Surface *img_surf = IMG_Load("resrc/images/folder_icon.png");
        data_ptr->directory_icon = SDL_CreateTextureFromSurface(renderer, img_surf);
        img_surf = IMG_Load("resrc/images/image_icon.png");
        data_ptr->image_icon = SDL_CreateTextureFromSurface(renderer, img_surf);
        img_surf = IMG_Load("resrc/images/right_arrow_icon.png");
        data_ptr->right_arrow_icon = SDL_CreateTextureFromSurface(renderer, img_surf);
        img_surf = IMG_Load("resrc/images/left_arrow_icon.png");
        data_ptr->left_arrow_icon = SDL_CreateTextureFromSurface(renderer, img_surf);
        img_surf = IMG_Load("resrc/images/basic_file_icon.png");
        data_ptr->file_icon = SDL_CreateTextureFromSurface(renderer, img_surf);
        img_surf = IMG_Load("resrc/images/code_icon.png");
        data_ptr->code_icon = SDL_CreateTextureFromSurface(renderer, img_surf);
        img_surf = IMG_Load("resrc/images/executable_icon.png");
        data_ptr->executable_icon = SDL_CreateTextureFromSurface(renderer, img_surf);
        img_surf = IMG_Load("resrc/images/video_icon.png");
        data_ptr->video_icon = SDL_CreateTextureFromSurface(renderer, img_surf);
        SDL_FreeSurface(img_surf);
    }
}

void render(SDL_Renderer *renderer, AppData *data_ptr)
{
    // erase renderer content
    SDL_SetRenderDrawColor(renderer, 235, 235, 235, 255);
    SDL_RenderClear(renderer);
    

    // TODO: draw!
    //SDL_RenderCopy(renderer, data_ptr->penguin, NULL, &rect);
    //int curr_x = 60;
    //int curr_y = 10;

    //int icon_y = 10;
    for (int i = data_ptr->page * 10; i < data_ptr->page * 10 + 10; i++) {
        if (i >= data_ptr->files.size()) {
            SDL_RenderPresent(renderer);
            return;
        }
        /*
        data_ptr->files_rect.push_back(new SDL_Rect);
        data_ptr->icon_rect.push_back(new SDL_Rect);
        data_ptr->icon_rect[i]->x = 10;
        data_ptr->icon_rect[i]->y = icon_y;
        data_ptr->icon_rect[i]->w = 40;
        data_ptr->icon_rect[i]->h = 40;
        //SDL_Rect rect = *data_ptr->files_rect[i];
        data_ptr->files_rect[i]->x = curr_x;
        data_ptr->files_rect[i]->y = curr_y;
        //printf("%d, %d\n", data_ptr->files_rect[i]->x, data_ptr->files_rect[i]->y);
        */
        //printf("%d, %d, %d, %d", data_ptr->icon_rect[i]->x, data_ptr->icon_rect[i]->y, data_ptr->icon_rect[i]->w, data_ptr->icon_rect[i]->h);
        if (data_ptr->file_sizes[i] == -1) {
            //Directory
            SDL_RenderCopy(renderer, data_ptr->directory_icon, NULL, &(*data_ptr->icon_rect[i]));
        } else {
            std::string path = data_ptr->current_directory + std::string("/") + std::string(data_ptr->files[i]);
            int returnval = 0;
            returnval = access(path.c_str(), X_OK);
            //Check if execute permissions
            if (returnval == 0) {
                SDL_RenderCopy(renderer, data_ptr->executable_icon, NULL, &(*data_ptr->icon_rect[i]));
            } else {
                if (data_ptr->files[i].find(std::string(".jpg")) != std::string::npos || data_ptr->files[i].find(std::string(".jpeg")) != std::string::npos ||
                data_ptr->files[i].find(std::string(".png")) != std::string::npos || data_ptr->files[i].find(std::string(".tif")) != std::string::npos ||
                data_ptr->files[i].find(std::string(".tiff")) != std::string::npos || data_ptr->files[i].find(std::string(".gif")) != std::string::npos) {
                    //Image
                    SDL_RenderCopy(renderer, data_ptr->image_icon, NULL, &(*data_ptr->icon_rect[i]));
                } else if (data_ptr->files[i].find(std::string(".mp4")) != std::string::npos || data_ptr->files[i].find(std::string(".mov")) != std::string::npos ||
                data_ptr->files[i].find(std::string(".mkv")) != std::string::npos || data_ptr->files[i].find(std::string(".avi")) != std::string::npos ||
                data_ptr->files[i].find(std::string(".webm")) != std::string::npos) {
                    //Video
                    SDL_RenderCopy(renderer, data_ptr->video_icon, NULL, &(*data_ptr->icon_rect[i]));
                } else if (data_ptr->files[i].find(std::string(".h")) != std::string::npos || data_ptr->files[i].find(std::string(".c")) != std::string::npos ||
                data_ptr->files[i].find(std::string(".cpp")) != std::string::npos || data_ptr->files[i].find(std::string(".py")) != std::string::npos ||
                data_ptr->files[i].find(std::string(".java")) != std::string::npos || data_ptr->files[i].find(std::string(".js")) != std::string::npos) {
                    //Code File
                    SDL_RenderCopy(renderer, data_ptr->code_icon, NULL, &(*data_ptr->icon_rect[i]));
                } else {
                    //Other
                    SDL_RenderCopy(renderer, data_ptr->file_icon, NULL, &(*data_ptr->icon_rect[i]));
                }
            }
        }
        SDL_QueryTexture(data_ptr->files_textures[i], NULL, NULL, &(data_ptr->files_rect[i]->w), &(data_ptr->files_rect[i]->h));
        SDL_RenderCopy(renderer, data_ptr->files_textures[i], NULL, data_ptr->files_rect[i]);
        if (data_ptr->file_sizes[i] != -1) {
            SDL_QueryTexture(data_ptr->file_sizes_textures[i], NULL, NULL, &(data_ptr->sizes_rect[i]->w), &(data_ptr->sizes_rect[i]->h));
            SDL_RenderCopy(renderer, data_ptr->file_sizes_textures[i], NULL, data_ptr->sizes_rect[i]);
        }
        SDL_QueryTexture(data_ptr->permissions_textures[i], NULL, NULL, &(data_ptr->permissions_rect[i]->w), &(data_ptr->permissions_rect[i]->h));
        SDL_RenderCopy(renderer, data_ptr->permissions_textures[i], NULL, data_ptr->permissions_rect[i]);

        //What is this section
        SDL_RenderCopy(renderer, data_ptr->left_arrow_icon, NULL, &(data_ptr->left_arrow_rect));
        data_ptr->left_arrow_rect.x = 250;
        data_ptr->left_arrow_rect.y = 525;
        data_ptr->left_arrow_rect.w = 40;
        data_ptr->left_arrow_rect.h = 40;
        SDL_RenderCopy(renderer, data_ptr->right_arrow_icon, NULL, &(data_ptr->right_arrow_rect));
        data_ptr->right_arrow_rect.x = 450;
        data_ptr->right_arrow_rect.y = 525;
        data_ptr->right_arrow_rect.w = 40;
        data_ptr->right_arrow_rect.h = 40;

    }

    // show rendered frame
    SDL_RenderPresent(renderer);
}

void quit(AppData *data_ptr)
{
    SDL_DestroyTexture(data_ptr->directory_icon);

    //Destroy each file's texture
    int size = data_ptr->files_textures.size();
    for (int i = 0; i < size; i++) {
        SDL_DestroyTexture(data_ptr->files_textures[i]);
    }
    TTF_CloseFont(data_ptr->font);
}

std::string clickedCheck(AppData *data_ptr, SDL_Event *event) {
    int i = 0;
    //printf("%d : %d\n", event->button.x, event->button.y);
    for (int i = data_ptr->page * 10; i < data_ptr->page * 10 + 10; i++) {
        if (i >= data_ptr->files.size()) {
            return "clicked empty space";
        }
        //printf("%d :  %d %d %d %d\n",i, data_ptr->files_rect[i]->x, data_ptr->files_rect[i]->y, data_ptr->files_rect[i]->h, data_ptr->files_rect[i]->w );
        if (event->button.button == SDL_BUTTON_LEFT &&
            event->button.x >= data_ptr->files_rect[i]->x &&
            event->button.x <= data_ptr->files_rect[i]->x + data_ptr->files_rect[i]->w &&
            event->button.y >= data_ptr->files_rect[i]->y &&
            event->button.y <= data_ptr->files_rect[i]->y + data_ptr->files_rect[i]->h)
        {
            if (data_ptr->files[i] == "..") {
                data_ptr->current_directory = data_ptr->previous_directory;
            }
            return data_ptr->files[i];
        }
        else if (event->button.button == SDL_BUTTON_LEFT &&
            event->button.x >= data_ptr->right_arrow_rect.x &&
            event->button.x <= data_ptr->right_arrow_rect.x + data_ptr->right_arrow_rect.w &&
            event->button.y >= data_ptr->right_arrow_rect.y &&
            event->button.y <= data_ptr->right_arrow_rect.y + data_ptr->right_arrow_rect.h && 
            data_ptr->files.size() >= (data_ptr->page + 1) * 10)
        {
            data_ptr->page = data_ptr->page + 1;
            //clickedOnDirectory(data_ptr);
            return "RIGHT ARROW";
        }
        else if (event->button.button == SDL_BUTTON_LEFT &&
            event->button.x >= data_ptr->left_arrow_rect.x &&
            event->button.x <= data_ptr->left_arrow_rect.x + data_ptr->left_arrow_rect.w &&
            event->button.y >= data_ptr->left_arrow_rect.y &&
            event->button.y <= data_ptr->left_arrow_rect.y + data_ptr->left_arrow_rect.h && 
            data_ptr->page - 1 >= 0)
        {
            data_ptr->page = data_ptr->page - 1;
            //clickedOnDirectory(data_ptr);
            return "LEFT ARROW";
        }
    }
    printf("clicked on empty space\n");
    return "help";
}


void clickedOnDirectory(AppData *data_ptr) {
    data_ptr->files_displayed.erase(data_ptr->files_displayed.begin(), data_ptr->files_displayed.end());
    int curr_x = 60;
    int curr_y = 10;
    int icon_y = 10;
    for (int i = data_ptr->page * 10; i < data_ptr->page * 10 + 10; i++) {
        data_ptr->files_displayed.push_back(data_ptr->files[i]);
        if (i >= data_ptr->files.size()) {
            return;
        }
        data_ptr->files_rect.push_back(new SDL_Rect);
        data_ptr->icon_rect.push_back(new SDL_Rect);
        data_ptr->sizes_rect.push_back(new SDL_Rect);
        data_ptr->permissions_rect.push_back(new SDL_Rect);
        data_ptr->icon_rect[i]->x = 10;
        data_ptr->icon_rect[i]->y = icon_y;
        data_ptr->icon_rect[i]->w = 40;
        data_ptr->icon_rect[i]->h = 40;
        //SDL_Rect rect = *data_ptr->files_rect[i];
        data_ptr->files_rect[i]->x = curr_x;
        data_ptr->files_rect[i]->y = curr_y;   
        //printf("%d, %d, %d, %d", data_ptr->icon_rect[i]->x, data_ptr->icon_rect[i]->y, data_ptr->icon_rect[i]->w, data_ptr->icon_rect[i]->h);

        data_ptr->sizes_rect[i]->x = 550;
        data_ptr->sizes_rect[i]->y = curr_y;

        data_ptr->permissions_rect[i]->x = 700;
        data_ptr->permissions_rect[i]->y = curr_y;

        curr_y = curr_y + 50;
        icon_y = icon_y + 50;
    }   
        return;
}


void updateFileList(SDL_Renderer *renderer, AppData *data_ptr) {
    std::cout << "-----------------------------" << std::endl;
    data_ptr->files.erase(data_ptr->files.begin(), data_ptr->files.end());
    data_ptr->file_sizes.erase(data_ptr->file_sizes.begin(), data_ptr->file_sizes.end());
    data_ptr->files_textures.erase(data_ptr->files_textures.begin(), data_ptr->files_textures.end());
    data_ptr->file_sizes_textures.erase(data_ptr->file_sizes_textures.begin(), data_ptr->file_sizes_textures.end());
    data_ptr->permissions_textures.erase(data_ptr->permissions_textures.begin(), data_ptr->permissions_textures.end());
    data_ptr->files_rect.erase(data_ptr->files_rect.begin(), data_ptr->files_rect.end());
    data_ptr->sizes_rect.erase(data_ptr->sizes_rect.begin(), data_ptr->sizes_rect.end());
    data_ptr->icon_rect.erase(data_ptr->icon_rect.begin(), data_ptr->icon_rect.end());
    data_ptr->permissions_rect.erase(data_ptr->permissions_rect.begin(), data_ptr->permissions_rect.end());
    
    DIR* dir = opendir(data_ptr->current_directory.c_str());
    struct dirent *entry;
    int q = 0;
    while ((entry = readdir(dir)) != NULL) {
        //if (q > data_ptr->page*10 && q < data_ptr->page*10+10) {
        data_ptr->files.push_back(entry->d_name);
        //}
        //q++;
        //data_ptr->files.push_back(entry->d_name);
    }
    
    closedir(dir);

    std::sort(data_ptr->files.begin(), data_ptr->files.end());
    if (!data_ptr->files[0].compare(std::string("."))) {
        data_ptr->files.erase(data_ptr->files.begin());
    }

    struct stat file_info;
    for (int i = 0; i < data_ptr->files.size(); i++) {
        SDL_Color color = { 0, 0, 0 };
        int file_err;
        //std::cout << "CURRENT DIRECTORY: " << data_ptr->current_directory.c_str() << std::endl;
        std::string full_path = std::string(data_ptr->current_directory.c_str()) + std::string("/") + data_ptr->files[i];
        file_err = stat(full_path.c_str(), &file_info);
        if (S_ISDIR(file_info.st_mode)) {
            //printf("%s (directory)\n", data_ptr->files[i].c_str());
            data_ptr->file_sizes.push_back(-1);
        }
        else {
            printf("%s (%ld bytes)\n", data_ptr->files[i].c_str(), file_info.st_size);
            data_ptr->file_sizes.push_back(file_info.st_size);
            std::string file_permissions = std::string(( (file_info.st_mode & S_IRUSR) ? "r" : "-")) + std::string(( (file_info.st_mode & S_IWUSR) ? "w" : "-"))
            + std::string(( (file_info.st_mode & S_IXUSR) ? "x" : "-")) + std::string(( (file_info.st_mode & S_IRGRP) ? "r" : "-")) +
            std::string(( (file_info.st_mode & S_IWGRP) ? "w" : "-")) + std::string(( (file_info.st_mode & S_IXGRP) ? "x" : "-")) +
            std::string(( (file_info.st_mode & S_IROTH) ? "r" : "-")) + std::string(( (file_info.st_mode & S_IWOTH) ? "w" : "-")) +
            std::string(( (file_info.st_mode & S_IXOTH) ? "x" : "-"));
        }
        SDL_Surface *phrase_surf = TTF_RenderText_Solid(data_ptr->font, data_ptr->files[i].c_str(), color);
        SDL_Texture *file = SDL_CreateTextureFromSurface(renderer, phrase_surf);
        data_ptr->files_textures.push_back(file);
        SDL_FreeSurface(phrase_surf);

        std::string suffix = std::string(" B");
            if (data_ptr->file_sizes[i] >= 1024) {
                suffix = std::string(" KB");
                data_ptr->file_sizes[i] = data_ptr->file_sizes[i] / 1024;
            }
            if (data_ptr->file_sizes[i] >= 1024) {
                suffix = std::string(" KB");
                data_ptr->file_sizes[i] = data_ptr->file_sizes[i] / 1024;
            }
            if (data_ptr->file_sizes[i] >= 1024) {
                suffix = std::string(" MB");
                data_ptr->file_sizes[i] = data_ptr->file_sizes[i] / 1024;
            }
            if (data_ptr->file_sizes[i] >= 1024) {
                suffix = std::string(" GB");
                data_ptr->file_sizes[i] = data_ptr->file_sizes[i] / 1024;
            }
            std::string file_size1 = std::to_string(data_ptr->file_sizes[i]) + suffix;
        phrase_surf = TTF_RenderText_Solid(data_ptr->font, file_size1.c_str(), color);
        SDL_Texture *file_size = SDL_CreateTextureFromSurface(renderer, phrase_surf);
        data_ptr->file_sizes_textures.push_back(file_size);
        SDL_FreeSurface(phrase_surf);

        std::string file_permissions = std::string(( (file_info.st_mode & S_IRUSR) ? "r" : "-")) + std::string(( (file_info.st_mode & S_IWUSR) ? "w" : "-"))
            + std::string(( (file_info.st_mode & S_IXUSR) ? "x" : "-")) + std::string(( (file_info.st_mode & S_IRGRP) ? "r" : "-")) +
            std::string(( (file_info.st_mode & S_IWGRP) ? "w" : "-")) + std::string(( (file_info.st_mode & S_IXGRP) ? "x" : "-")) +
            std::string(( (file_info.st_mode & S_IROTH) ? "r" : "-")) + std::string(( (file_info.st_mode & S_IWOTH) ? "w" : "-")) +
            std::string(( (file_info.st_mode & S_IXOTH) ? "x" : "-"));
        phrase_surf = TTF_RenderText_Solid(data_ptr->font, file_permissions.c_str(), color);
        SDL_Texture *file_permissions_texture = SDL_CreateTextureFromSurface(renderer, phrase_surf);
        data_ptr->permissions_textures.push_back(file_permissions_texture);
        SDL_FreeSurface(phrase_surf);

        //std::cout << "NAME: " << data_ptr->files[i] << ", SIZE: " << data_ptr->file_sizes[i];
    }
    SDL_Surface *img_surf = IMG_Load("resrc/images/folder_icon.png");
    data_ptr->directory_icon = SDL_CreateTextureFromSurface(renderer, img_surf);
    img_surf = IMG_Load("resrc/images/image_icon.png");
    data_ptr->image_icon = SDL_CreateTextureFromSurface(renderer, img_surf);
    img_surf = IMG_Load("resrc/images/right_arrow_icon.png");
    data_ptr->right_arrow_icon = SDL_CreateTextureFromSurface(renderer, img_surf);
    img_surf = IMG_Load("resrc/images/left_arrow_icon.png");
    data_ptr->left_arrow_icon = SDL_CreateTextureFromSurface(renderer, img_surf);
    img_surf = IMG_Load("resrc/images/basic_file_icon.png");
    data_ptr->file_icon = SDL_CreateTextureFromSurface(renderer, img_surf);
    img_surf = IMG_Load("resrc/images/code_icon.png");
    data_ptr->code_icon = SDL_CreateTextureFromSurface(renderer, img_surf);
    img_surf = IMG_Load("resrc/images/executable_icon.png");
    data_ptr->executable_icon = SDL_CreateTextureFromSurface(renderer, img_surf);
    img_surf = IMG_Load("resrc/images/video_icon.png");
    data_ptr->video_icon = SDL_CreateTextureFromSurface(renderer, img_surf);
    SDL_FreeSurface(img_surf);   
}