#include <cstdio>
#include <sstream>
#include <dirent.h>

#include <pspiofilemgr.h>

#include "browser.h"
#include "gamemgr.h"
#include "system_mgr.h"
#include "osk.h"

#include "iso.h"
#include "cso.h"
#include "eboot.h"
#include "unziprar.h"

#define INITIAL_DIR "ms0:/" // Initial directory
#define GO_DIR "ef0:/" // PSP Go initial directory
#define PAGE_SIZE 10 // maximum entries shown on screen
#define BUF_SIZE 1024*16 // 16 kB buffer for copying files

#define MAX_SCROLL_TIME 50

#include "browser_entries.h"

static Browser* self;

#define MAX_OPTIONS 9
static const struct {
        int x;
        int y;
        char* name;
} pEntries[MAX_OPTIONS] = {
    {10, 80, "Cancel"},
    {10, 100, "Copy"},
    {10, 120, "Cut"},
    {10, 140, "Paste"},
    {10, 160, "Delete"},
    {10, 180, "Rename"},
    {10, 200, "New Dir"},
    {10, 220, "Go to ms0:/"},
    {10, 240, "Go to ef0:/"},
};

Browser::Browser(){
    self = this;
    this->cwd = GO_DIR; // current working directory (cwd)
    this->entries = new vector<Entry*>(); // list of files and folders in cwd
    this->pasteMode = NO_MODE;
    this->index = 0;
    this->start = 0;
    this->animating = false;
    this->moving = 0;
    this->enableSelection = true;
    this->selectedBuffer = new vector<string>(); // list of paths to paste
    this->draw_progress = false;
    this->optionsmenu = NULL;
    this->checkBox = new Image(PKG_PATH, YA2D_PLACE_VRAM, common::findPkgOffset("CHECK.PNG"));
    this->uncheckBox = new Image(PKG_PATH, YA2D_PLACE_VRAM, common::findPkgOffset("UNCHECK.PNG"));
    this->folderIcon = new Image(PKG_PATH, YA2D_PLACE_VRAM, common::findPkgOffset("FOLDER.PNG"));
    this->fileIcon = new Image(PKG_PATH, YA2D_PLACE_VRAM, common::findPkgOffset("FILE.PNG"));
    
    this->optionsDrawState = 0;
    this->optionsAnimX = 0;
    this->optionsAnimY = 0;
    this->pEntryIndex = 0;
    this->refreshDirs();
}

Browser::~Browser(){
    this->entries->clear();
    this->selectedBuffer->clear();
    delete this->entries;
    delete this->selectedBuffer;
}

void Browser::moveDirUp(){
    // Move to the parent directory of this->cwd
    if (this->cwd == INITIAL_DIR || this->cwd == GO_DIR)
        return;
    size_t lastSlash = this->cwd.rfind("/", this->cwd.rfind("/", string::npos)-1);
    this->cwd = this->cwd.substr(0, lastSlash+1);
    this->refreshDirs();
}
        
void Browser::update(){
    // Move to the next directory pointed by the currently selected entry or run an app if selected file is one
    if (this->entries->size() == 0)
        return;
    common::playMenuSound();
    if (this->get()->getName() == "./")
        refreshDirs();
    else if (this->get()->getName() == "../")
        moveDirUp();
    else if (string(this->get()->getType()) == "FOLDER"){
        this->cwd = this->get()->getPath();
        this->refreshDirs();
    }
    else if (Iso::isISO(this->get()->getPath().c_str())){
        Iso* iso = new Iso(this->get()->getPath());
        iso->execute();
    }
    else if (Cso::isCSO(this->get()->getPath().c_str())){
        Cso* cso = new Cso(this->get()->getPath());
        cso->execute();
    }
    else if (Eboot::isEboot(this->get()->getPath().c_str())){
        Eboot* eboot = new Eboot(this->get()->getPath());
        eboot->execute();
    }
    else if (Entry::isZip(this->get()->getPath().c_str())){
        extractArchive(0);
    }
    else if (Entry::isRar(this->get()->getPath().c_str())){
        extractArchive(1);
    }
}

void Browser::extractArchive(int type){

    string name = this->get()->getName();
    string dest = this->cwd + name.substr(0, name.rfind('.')) + "/";
    sceIoMkdir(dest.c_str(), 0777);

    progress_desc[0] = "Extracting archive";
    progress_desc[1] = "    "+name;
    progress_desc[2] = "into";
    progress_desc[3] = "    "+dest;
    progress_desc[4] = "Please Wait";
    
    bool noRedraw = draw_progress;
    if (!noRedraw)
        draw_progress = true;
    
    if (type)
        DoExtractRAR(this->get()->getPath().c_str(), dest.c_str(), NULL);
    else
        unzipToDir(this->get()->getPath().c_str(), dest.c_str(), NULL);
    
    if (!noRedraw)
        draw_progress = false;
    
    this->refreshDirs();
    
    //GameManager::updateGameList(dest.c_str()); // tell GameManager to update if needed
}

void Browser::refreshDirs(){

    DIR* dir = opendir(this->cwd.c_str());
    if (dir == NULL){ // can't open directory
        if (this->cwd == INITIAL_DIR) // ms0 failed
            this->cwd = GO_DIR; // go to ef0
        else
            this->cwd = INITIAL_DIR;
        refreshDirs();
        return;
    }

    // Refresh the list of files and dirs
    SystemMgr::pauseDraw();
    this->index = 0;
    this->start = 0;
    this->entries->clear();
    this->animating = false;
    this->draw_progress = false;
    this->optionsmenu = NULL;
    SystemMgr::resumeDraw();

    struct dirent* dit;

    vector<Entry*> folders;
    vector<Entry*> files;
        
    while ((dit = readdir(dir))){
        if (common::fileExists(string(this->cwd)+string(dit->d_name)))
            files.push_back(new File(string(this->cwd)+string(dit->d_name)));
        else
            folders.push_back(new Folder(string(this->cwd)+string(dit->d_name)+"/"));
    }
    closedir(dir);
    
    SystemMgr::pauseDraw();
    for (int i=0; i<folders.size(); i++)
        entries->push_back(folders.at(i));
    for (int i=0; i<files.size(); i++)
        entries->push_back(files.at(i));
    SystemMgr::resumeDraw();
    
    if (this->entries->size() == 0){
        this->cwd = INITIAL_DIR;
        refreshDirs();
        return;
    }
}
        

void Browser::drawScreen(){

    const int xoffset = 165;
    int yoffset = 50;
    
    
    if (moving && entries->size() > 0){
        int height = 230/entries->size();
        int x = xoffset-65;
        int y = yoffset-20;
        common::getImage(IMAGE_DIALOG)->draw_scale(x, y + (index*height), 5, height);
    }
    common::getImage(IMAGE_DIALOG)->draw_scale(xoffset-50, yoffset-20, 360, 230);
    
    if (entries->size() == 0){
        Image* img = common::getImage(IMAGE_WAITICON);
        img->draw((480-img->getTexture()->width)/2, (272-img->getTexture()->height)/2);
        return;
    }
    
    for (int i=this->start; i<min(this->start+PAGE_SIZE, (int)entries->size()); i++){
        File* e = (File*)this->entries->at(i);
        
        if(e->isSelected()){
            this->checkBox->draw(xoffset-30, yoffset-10);
        }else{
            this->uncheckBox->draw(xoffset-30, yoffset-10);
        }
        if (i == index && this->enableSelection){
            if (animating){
                common::printText(xoffset, yoffset, e->getName().c_str(), LITEGRAY, SIZE_MEDIUM, true, true);
                animating = false;
            }
            else
                common::printText(xoffset, yoffset, e->getName().c_str(), LITEGRAY, SIZE_BIG, true, true);
        }
        else{
            common::printText(xoffset, yoffset, this->formatText(e->getName()).c_str());
        }
        common::printText(400, yoffset, e->getSize().c_str());
        if (string(e->getType()) == "FOLDER")
            this->folderIcon->draw(xoffset-15, yoffset-10);
        else
            this->fileIcon->draw(xoffset-15, yoffset-10);
        yoffset += 20;
    }
}

void Browser::drawProgress(){
    if (!draw_progress /*|| progress>=max_progress*/)
        return;
        
    int w = min(480, 10*common::maxString(progress_desc, 5));
    int h = 30 + 15*4;
    int x = (480-w)/2;
    int y = (272-h)/2;
    common::getImage(IMAGE_DIALOG)->draw_scale(x, y, w, h);
    
    int yoffset = y+10;
    for (int i=0; i<5; i++){
        if (i==4 && progress_desc[4] == ""){
            ostringstream s;
            s<<progress<<" / "<<max_progress;  
            common::printText(x+20, yoffset, s.str().c_str());  
        }
        else common::printText(x+20, yoffset, progress_desc[i].c_str());
        yoffset+=15;
    }
    
}    

void Browser::draw(){
    this->drawScreen();
    this->drawOptionsMenu();
    this->drawProgress();
    if (this->optionsmenu != NULL)
        this->optionsmenu->draw();
}

string Browser::formatText(string text){
    // Format the text shown, text with more than 13 characters will be truncated and ... be appended to the name
    if (text.length() <= 25)
        return text;
    else{
        string* ret = new string(text.substr(0, 22));
        *ret += "...";
        return *ret;
    }
}
        
void Browser::select(){
    // Select or unselect the entry pointed by the cursor
    if (this->entries->size() == 0)
        return;
    Folder* e = (Folder*)this->get();
    if (string(e->getName()) == "./")
        return;
    else if (string(e->getName()) == "../")
        return;
    e->changeSelection();
}

Entry* Browser::get(){
    // Obtain the currectly selected entry, this will return the instance of the entry, not it's name
    return this->entries->at(this->index);
}
        
void Browser::down(){
    // Move the cursor down, this updates index and page
    if (this->entries->size() == 0)
        return;
    this->moving = MAX_SCROLL_TIME;
    if (this->index == (entries->size()-1)){
        this->index = 0;
        this->start = 0;
    }
    else if (this->index-this->start == PAGE_SIZE-1){
        if (this->index+1 < entries->size())
            this->index++;
        if (this->start+PAGE_SIZE < entries->size())
            this->start++;
    }
    else if (this->index+1 < entries->size())
        this->index++;
    this->animating = true;
    common::playMenuSound();
}
        
void Browser::up(){
    // Move the cursor up, this updates index and page
    if (this->entries->size() == 0)
        return;
    this->moving = MAX_SCROLL_TIME;
    if (this->index == 0){
        this->index = entries->size()-1;
        this->start = entries->size() - PAGE_SIZE;
        if (this->start < 0) this->start = 0;
    }
    else if (this->index == this->start){
        this->index--;
        if (this->start>0)
            this->start--;
    }
    else
        this->index--;
    this->animating = true;
    common::playMenuSound();
}

void Browser::recursiveFolderDelete(string path){
        //try to open directory
    SceUID d = sceIoDopen(path.c_str());
    
    if(d >= 0)
    {
        SceIoDirent entry;
        memset(&entry, 0, sizeof(SceIoDirent));
        
        //allocate memory to store the full file paths
        char * new_path = new char[path.length() + 256];

        //start reading directory entries
        while(sceIoDread(d, &entry) > 0)
        {
            //skip . and .. entries
            if (!strcmp(".", entry.d_name) || !strcmp("..", entry.d_name)) 
            {
                memset(&entry, 0, sizeof(SceIoDirent));
                continue;
            };
            
            //build new file path
            strcpy(new_path, path.c_str());
            strcat(new_path, entry.d_name);
            
            if(common::fileExists(new_path))
                self->deleteFile(new_path);
            else {
                //not a file? must be a folder
                strcat(new_path, "/");
                recursiveFolderDelete(string(new_path)); //try to delete folder content
            }
            
        };
        
        sceIoDclose(d); //close directory
        sceIoRmdir(path.substr(0, path.length()-1).c_str()); //delete empty folder

        delete [] new_path; //clear allocated memory
    };
}

void Browser::deleteFolder(string path){
    // Recursively delete the path
    
    progress_desc[0] = "Deleting folder";
    progress_desc[1] = "    "+path;
    progress_desc[2] = "";
    progress_desc[3] = "";
    progress_desc[4] = "Please Wait";
    
    bool noRedraw = draw_progress;
    if (!noRedraw)
        draw_progress = true;
    
     //protect some folders
    if(path == "ms0:/PSP/" || path == "ms0:/PSP/GAME/" || path == "ms0:/PSP/LICENSE/"
            || path == "ef0:/PSP/" || path == "ef0:/PSP/GAME/" || path == "ef0:/PSP/LICENSE/")
        return;

    recursiveFolderDelete(path);

    if (!noRedraw)
        draw_progress = false;
}

void Browser::deleteFile(string path){
    progress_desc[0] = "Deleting file";
    progress_desc[1] = "    "+path;
    progress_desc[2] = "";
    progress_desc[3] = "";
    progress_desc[4] = "Please Wait";
    
    bool noRedraw = draw_progress;
    if (!noRedraw)
        draw_progress = true;
    
    sceIoRemove(path.c_str());
    
    if (!noRedraw)
        draw_progress = false;
}

int Browser::copy_folder_recursive(const char * source, const char * destination)
{

    draw_progress = true;

    //create new folder
    sceIoMkdir(destination, 0777);
    
    char * new_destination = new char[strlen(destination) + 256];
    strcpy(new_destination, destination);
    strcat(new_destination, "/");
    
    char* entry_copy = new_destination + strlen(destination) + 1;
    
    //try to open source folder
    SceUID dir = sceIoDopen(source);
    
    if(dir >= 0)
    {
        SceIoDirent entry;
        memset(&entry, 0, sizeof(SceIoDirent));
        
        char * read_path = new char[strlen(source) + 256];
        
        //start reading directory entries
        while(sceIoDread(dir, &entry) > 0)
        {
            //skip . and .. entries
            if (!strcmp(".", entry.d_name) || !strcmp("..", entry.d_name)) 
            {
                memset(&entry, 0, sizeof(SceIoDirent));
                continue;
            };
        
            //build read path
            strcpy(read_path, source);
            strcat(read_path, "/");
            strcat(read_path, entry.d_name);
        
            //pspDebugScreenPrintf("file copy from: %s\n", read_path);
        
            strcpy(entry_copy, entry.d_name); //new file

            //pspDebugScreenPrintf("to %s\n", new_destination);
            
            if (common::fileExists(read_path)) //is it a file
                copyFile(string(read_path), string(destination)+string("/")); //copy file
            else
            {
                //try to copy as a folder
                //strcat(new_destination, "/");
                //strcat(read_path, "/");
                copy_folder_recursive(read_path, new_destination);
            };

        };
        
        delete [] read_path;
        
        //close folder
        sceIoDclose(dir);
    };
    
    //free allocated path
    delete [] new_destination;
    
    draw_progress = false;
    
    return 1;
};

string Browser::checkDestExists(string path, string destination, string name){
    string dest = destination+name;
    int copies = 1;
    int src_equals_dest = ( (path==dest) || (path == dest+"/") );
    while (common::fileExists(dest) || common::folderExists(dest)){
        char* description = "Destination exists, what to do?";
        t_options_entry options_entries[3] = {
            {OPTIONS_CANCELLED, "Cancel"}, {1, "Rename destination"}, {0, "Overwrite"}
        };
        optionsmenu = new OptionsMenu(description, (src_equals_dest)? 2 : 3, options_entries);
        int ret = optionsmenu->control();
        OptionsMenu* aux = optionsmenu;
        optionsmenu = NULL;
        delete aux;
        
        switch (ret){
        case 0:
             if (common::fileExists(dest))
                deleteFile(dest);
             else
                deleteFolder(dest+"/");
             break;
        case 1:
            do{
                stringstream ss;
                ss << destination << '(' << copies++ << ')' << name;
                dest = ss.str();
            }while (common::fileExists(dest) || common::folderExists(dest));
            return dest;
        default: return "";
        }
    }
    return dest;
}


void Browser::copyFolder(string path){
    // Copy the folder into cwd

    if(path == this->cwd)
        return;
    
    if(!strncmp(path.c_str(), this->cwd.c_str(), path.length())) //avoid inception
        return;
    
    Folder* f = new Folder(path);
    
    string destination = checkDestExists(path, this->cwd, f->getName().substr(0, f->getName().length()-1));
    
    if (destination.size() == 0) return; // copy cancelled
    
    copy_folder_recursive(path.substr(0, path.length()-1).c_str(), destination.c_str());
}

void Browser::copyFile(string path, string destination){
    size_t lastSlash = path.rfind("/", string::npos);
    string name = path.substr(lastSlash+1, string::npos);
    string dest = checkDestExists(path, destination, name);
    
    if (dest.size() == 0) return; // copy canceled
    
    SceUID src = sceIoOpen(path.c_str(), PSP_O_RDONLY, 0777);
    SceUID dst = sceIoOpen(dest.c_str(), PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
    
    progress_desc[0] = "Copying file";
    progress_desc[1] = "    "+path;
    progress_desc[2] = "into";
    progress_desc[3] = "    "+dest;
    progress_desc[4] = "";
    
    progress = 0;
    max_progress = sceIoLseek(src, 0, SEEK_END);
    sceIoLseek(src, 0, SEEK_SET);

    bool noRedraw = draw_progress;
    if (!noRedraw)    
        draw_progress = true;
    
    int read;
    u8* buffer = new u8[BUF_SIZE];
    
    do {
        read = sceIoRead(src, buffer, BUF_SIZE);
        sceIoWrite(dst, buffer, read);
        progress += read;
    } while (read > 0 && progress < max_progress);
    sceIoClose(src);
    sceIoClose(dst);
    delete buffer;
    
    if (!noRedraw)
        draw_progress = false;
}

void Browser::copyFile(string path){
    copyFile(path, this->cwd);
}

void Browser::fillSelectedBuffer(){
    this->selectedBuffer->clear();
    for (int i=0; i<entries->size(); i++){
        File* e = (File*)entries->at(i);
        if (e->isSelected())
            this->selectedBuffer->push_back(e->getPath());
    }
}

void Browser::copy(){
    // Mark the paste mode as copy
    this->pasteMode = COPY;
    this->fillSelectedBuffer();
}

void Browser::cut(){
    // Mark the paste mode as cut
    this->pasteMode = CUT;
    this->fillSelectedBuffer();
}

static int pspIoMove(string src, string dest)
{
    if ( dest.find(src) != 0 )
    {
        const char deviceSize = 4;

        u32 data[2];
        data[0] = (u32)src.c_str() + deviceSize;
        data[1] = (u32)dest.c_str() + deviceSize;

        int res = sceIoDevctl("ms0:", 0x02415830, data, sizeof(data), NULL, 0);

        return res;
    }
    else    return -1;
}

void Browser::paste(){
    // Copy or cut all paths in the paste buffer to the cwd
    for (int i = 0; i<selectedBuffer->size(); i++){
        string e = selectedBuffer->at(i);
        if (common::fileExists(e)){
            this->copyFile(e);
            if (pasteMode == CUT) sceIoRemove(e.c_str());
        }
        else {
            this->copyFolder(e);
            if (pasteMode == CUT) this->deleteFolder(e);
        }
    }
    if (selectedBuffer->size()){
        this->refreshDirs();
        GameManager::updateGameList(cwd.c_str()); // tell GameManager to update
    }
    this->selectedBuffer->clear();
}

void Browser::rename(){
    SystemMgr::pauseDraw();
    string name = this->get()->getName();
    OSK osk;
    char* oldname = (char*)malloc(name.length());
    if (name.at(name.length()-1) == '/')
        strcpy(oldname, name.substr(0, name.length()-1).c_str());
    else
        strcpy(oldname, name.c_str());
    
    osk.init("New name for file/folder", oldname, 50);
    osk.loop();
    if(osk.getResult() != OSK_CANCEL)
    {
        char tmpText[51];
        osk.getText((char*)tmpText);
        sceIoRename((this->cwd+string(oldname)).c_str(), (this->cwd+string(tmpText)).c_str());
        GameManager::updateGameList(cwd.c_str()); // tell GameManager to update
    }
    osk.end();
    free(oldname);
    SystemMgr::resumeDraw();
    this->refreshDirs();
}

void Browser::removeSelection(){
    // Delete all paths in the paste buffer
    draw_progress = true;
    this->fillSelectedBuffer();
    if (this->selectedBuffer->size() == 0)
        this->selectedBuffer->push_back(this->get()->getPath());
        
    if (this->selectedBuffer->size() > 0){
        for (int i = 0; i<selectedBuffer->size(); i++){
            string e = selectedBuffer->at(i);
            if (common::fileExists(e))
                deleteFile(e);
            else
                this->deleteFolder(e);
        }
        this->selectedBuffer->clear();
        this->refreshDirs();
        GameManager::updateGameList(cwd.c_str()); // tell GameManager to update
    }
    draw_progress = false;
}

void Browser::makedir(){
    SystemMgr::pauseDraw();
    OSK osk;
    osk.init("Name of new directory", "new dir", 50);
    osk.loop();
    if(osk.getResult() != OSK_CANCEL)
    {
        char tmpText[51];
        osk.getText((char*)tmpText);
        string dirName = string(tmpText);
        sceIoMkdir((this->cwd+dirName).c_str(), 0777);
    }
    osk.end();
    SystemMgr::resumeDraw();
    this->refreshDirs();
}

void Browser::drawOptionsMenu(){

    switch (optionsDrawState){
        case 0:
            common::getImage(IMAGE_DIALOG)->draw_scale(0, 232, 40, 40);
            common::printText(5, 252, "...", GRAY_COLOR, 2.f);
            break;
        case 1: // draw opening animation
            common::getImage(IMAGE_DIALOG)->draw_scale(optionsAnimX, optionsAnimY, 132, 220);
            optionsAnimX += 20;
            optionsAnimY -= 40;
            if (optionsAnimX > 0)
                optionsDrawState = 2;
            break;
        case 2: // draw menu
            optionsAnimX = 0;
            optionsAnimY = 52;
            common::getImage(IMAGE_DIALOG)->draw_scale(0, 52, 132, 220);
        
            for (int i=0; i<MAX_OPTIONS; i++){
                if (i == pEntryIndex)
                    common::printText(pEntries[i].x, pEntries[i].y, pEntries[i].name, LITEGRAY, SIZE_BIG, true);
                else
                    common::printText(pEntries[i].x, pEntries[i].y, pEntries[i].name);
            }
            break;
        case 3: // draw closing animation
            common::getImage(IMAGE_DIALOG)->draw_scale(optionsAnimX, optionsAnimY, 132, 220);
            optionsAnimX -= 20;
            optionsAnimY += 40;
            if (optionsAnimX < -120)
                optionsDrawState = 0;
            break;
    }
}

void Browser::optionsMenu(){
    
    this->enableSelection = false;

    optionsAnimX = -100;
    optionsAnimY = 300;
    optionsDrawState = 1;
    while (optionsDrawState != 2)
        sceKernelDelayThread(0);

    Controller cont;
    Controller* pad = &cont;
    
    while (true){
        
        pad->update();
        
        if (pad->down()){
            common::playMenuSound();
            if (pEntryIndex < MAX_OPTIONS-1){
                pEntryIndex++;
            }
            else{
                pEntryIndex = 0;
            }
        }
        else if (pad->up()){
            common::playMenuSound();
            if (pEntryIndex > 0){        
                pEntryIndex--;
            }
            else{
                pEntryIndex = MAX_OPTIONS-1;
            }
        }
        else if (pad->decline() || pad->LT()){
            pEntryIndex = 0;
            break;
        }
        else if (pad->accept())
            break;
    }
    
    common::playMenuSound();
    
    optionsAnimX = 0;
    optionsAnimY = 52;
    optionsDrawState = 3;
    while (optionsDrawState != 0)
        sceKernelDelayThread(0);
    
    this->enableSelection = true;

    sceKernelDelayThread(100000);
}

void Browser::options(){
    // Run the system menu with the available browser options
    this->pEntryIndex = 0;
    this->optionsMenu();

    switch (pEntryIndex){
    case NO_MODE:                                                     break;
    case COPY:        this->copy();                                   break;
    case CUT:         this->cut();                                    break;
    case PASTE:       this->paste();                                  break;
    case DELETE:      this->removeSelection();                        break;
    case RENAME:      this->rename();                                 break;
    case MKDIR:       this->makedir();                                break;
    case MS0_DIR:     this->cwd = INITIAL_DIR; this->refreshDirs();   break;
    case EF0_DIR:     this->cwd = GO_DIR;      this->refreshDirs();   break;
    }
}
        
void Browser::control(Controller* pad){
    // Control the menu through user input
    if (pad->up())
        this->up();
    else if (pad->down())
        this->down();
    else if (pad->accept())
        this->update();
    else if (pad->decline()){
        common::playMenuSound();
        this->moveDirUp();
    }
    else if (pad->square()){
        common::playMenuSound();
        this->select();
    }
    else if (pad->LT()){
        common::playMenuSound();
        this->options();
    }
    else if (pad->select()){
        this->refreshDirs();
    }
    else{
        if (moving) moving--;
    }
}
