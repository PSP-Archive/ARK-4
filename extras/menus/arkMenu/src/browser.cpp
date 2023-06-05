#include <cstdio>
#include <sstream>
#include <dirent.h>
#include <algorithm>
#include <pspiofilemgr.h>

#include "browser.h"
#include "gamemgr.h"
#include "system_mgr.h"
#include "osk.h"

#include "iso.h"
#include "eboot.h"
#include "unziprar.h"
#include "texteditor.h"
#include "image_viewer.h"
#include "music_player.h"

#define ROOT_DIR "ms0:/" // Initial directory
#define GO_ROOT "ef0:/" // PSP Go initial directory
#define FTP_ROOT "ftp:/" // FTP directory
#define UMD_ROOT "disc0:/" // UMD directory
#define PAGE_SIZE 10 // maximum entries shown on screen
#define BUF_SIZE 1024*16 // 16 kB buffer for copying files

#define MAX_SCROLL_TIME 50

#include "browser_entries.h"

static Browser* self;

typedef BrowserFile File;
typedef BrowserFolder Folder;

extern "C" int kuKernelLoadModule(const char*, int, void*);

#define MAX_OPTIONS 12
static char* pEntries[MAX_OPTIONS] = {
    (char*) "Cancel",
    (char*) "Copy",
    (char*) "Cut",
    (char*) "Paste",
    (char*) "Delete",
    (char*) "Rename",
    (char*) "Create new",
    (char*) "Go to ms0:/",
    (char*) "Go to ef0:/",
    (char*) "Go to ftp:/",
    (char*) "Go to disc0:/",
};

BrowserDriver* Browser::ftp_driver = NULL;

Browser::Browser(){
    self = this;
    this->cwd = GO_ROOT; // current working directory (cwd)
    this->entries = new vector<Entry*>(); // list of files and folders in cwd
    this->pasteMode = NO_MODE;
    this->index = 0;
    this->start = 0;
    this->animating = false;
    this->moving = 0;
    this->enableSelection = true;
    this->clipboard = new vector<string>(); // list of paths to paste
    this->draw_progress = false;
    this->optionsmenu = NULL;

    this->hide_main_window = false;
    this->optionsDrawState = 0;
    this->optionsAnimX = 0;
    this->optionsAnimY = 0;
    this->pEntryIndex = 0;
    this->animation = 0;

    int psp_model = common::getPspModel();
    if (psp_model != PSP_GO){
        pEntries[EF0_DIR] = NULL;
    }
    else{
        pEntries[UMD_DIR] = NULL;
    }
    if (psp_model == PSP_11000 || ftp_driver == NULL){
        pEntries[FTP_DIR] = NULL;
    }
    if (IS_VITA(common::getArkConfig()) || psp_model == PSP_GO){
        pEntries[UMD_DIR] = NULL;
    }

    this->refreshDirs();
}

Browser::~Browser(){
    this->clearEntries();
    this->clipboard->clear();
    delete this->entries;
    delete this->clipboard;
}

const char* Browser::getCWD(){
    return self->cwd.c_str();
}

const char* getBrowserCWD(){
    return Browser::getCWD();
}

void Browser::clearEntries(){
    for (int i=0; i<entries->size(); i++){
        delete entries->at(i);
    }
    entries->clear();
}

bool Browser::isRootDir(string dir){
    return (dir == ROOT_DIR || dir == GO_ROOT || dir == FTP_ROOT || dir == UMD_ROOT);
}

void Browser::moveDirUp(){
    // Move to the parent directory of this->cwd
    if (isRootDir(this->cwd))
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
    else if (this->get()->getName() == "<refresh>"){
        this->refreshDirs();
    }
    else if (this->get()->getName() == "<disconnect>"){ // FTP disconnect entry
        if (ftp_driver != NULL) ftp_driver->disconnect();
        this->cwd = MS0_DIR;
        this->refreshDirs();
    }
    else if (string(this->get()->getType()) == "FOLDER"){
        this->cwd = this->get()->getPath();
        this->refreshDirs();
    }
    else if (Iso::isISO(this->get()->getPath().c_str())){
        if (this->cwd == "ms0:/ISO/VIDEO/" || this->cwd == "ef0:/ISO/VIDEO/")
            Iso::executeVideoISO(this->get()->getPath().c_str());
        else{
            Iso* iso = new Iso(this->get()->getPath());
            iso->execute();
        }
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
    else if (Entry::isPRX(this->get()->getPath().c_str())){
        installPlugin();
    }
	else if (Entry::isARK(this->get()->getPath().c_str())) {
		installTheme();
	}
    else if (Entry::isTXT(this->get()->getPath().c_str())){
        optionsmenu = new TextEditor(this->get()->getPath());
        optionsmenu->control();
        TextEditor* aux = (TextEditor*)optionsmenu;
        optionsmenu = NULL;
        delete aux;
    }
    else if (Entry::isIMG(this->get()->getPath().c_str())){
        optionsmenu = new ImageViewer(this->get()->getPath());
        optionsmenu->control();
        ImageViewer* aux = (ImageViewer*)optionsmenu;
        optionsmenu = NULL;
        delete aux;
    }
    else if (Entry::isMusic(this->get()->getPath().c_str())){
        this->hide_main_window = true;
        vector<string> selected;
        for (int i=0; i<entries->size(); i++){
            BrowserFile* e = (BrowserFile*)entries->at(i);
            if (e->isSelected() && e->getFileType() == FILE_MUSIC) selected.push_back(e->getPath());
        }
        BrowserFile* e = (BrowserFile*)get();
        if (!e->isSelected()) selected.push_back(e->getPath());
        if (selected.size() > 1){
            optionsmenu = new MusicPlayer(&selected);
        }
        else{
            optionsmenu = new MusicPlayer(this->get()->getPath());
        }
        optionsmenu->control();
        MusicPlayer* aux = (MusicPlayer*)optionsmenu;
        optionsmenu = NULL;
        delete aux;
        hide_main_window = false;
    }
}

void Browser::installTheme() {
	Entry* e = this->get();
	t_options_entry options_entries[] = {
		{OPTIONS_CANCELLED, "Cancel"},
		{0, "Preview"},
		{1, "Install"},
	};

	// Sanity checks
	if(e->getName() != "THEME.ARK" || e->getPath().substr(2) == "0:/PSP/SAVEDATA/ARK_01234/THEME.ARK") return;

	optionsmenu = new OptionsMenu("Install Theme", sizeof(options_entries)/sizeof(t_options_entry), options_entries);
	int ret = optionsmenu->control();
	OptionsMenu* aux = optionsmenu;
	optionsmenu = NULL;
	delete aux;

	if (ret == OPTIONS_CANCELLED) return;

    // load new theme
	SystemMgr::pauseDraw();
    printf("deleting current theme resources\n");
    common::deleteTheme();
    printf("set new theme path to: %s\n", e->getPath().c_str());
    common::setThemePath((char*)e->getPath().c_str());
    printf("loading new theme\n");
    common::loadTheme();
    SystemMgr::resumeDraw();
    printf("done\n");

	// Ask before overwriting theme
	if (ret == 0) {
        t_options_entry options_entries[] = {
            {OPTIONS_CANCELLED, "Cancel"},
            {0, "Accept Install"},
            {1, "Use theme without installing"},
        };
        optionsmenu = new OptionsMenu("Install Theme", sizeof(options_entries)/sizeof(t_options_entry), options_entries);
        int ret = optionsmenu->control();
        OptionsMenu* aux = optionsmenu;
        optionsmenu = NULL;
        delete aux;

        if (ret == 1){
            return;
        }
        else if (ret == OPTIONS_CANCELLED){
            // reset back to default theme
            SystemMgr::pauseDraw();
            common::deleteTheme();
            common::setThemePath();
            common::loadTheme();
            SystemMgr::resumeDraw();
            return;
        }
    }

    deleteFile(THEME_NAME);
    copyFile(e->getPath(), common::getArkConfig()->arkpath);
}

void Browser::installPlugin(){
    Entry* e = this->get();
    t_options_entry options_entries[] = {
        {OPTIONS_CANCELLED, "Cancel"},
        {0, "Always"},
        {1, "Game"},
        {2, "POPS (PS1)"},
        {3, "VSH (XMB)"},
        {4, "UMD/ISO"},
        {5, "Homebrew"},
        {6, "<Game ID>"},
        {7, "<LoadStart Module>"},
    };

    optionsmenu = new OptionsMenu("Install Plugin", sizeof(options_entries)/sizeof(t_options_entry), options_entries);
    int ret = optionsmenu->control();
    OptionsMenu* aux = optionsmenu;
    optionsmenu = NULL;
    delete aux;

    if (ret == OPTIONS_CANCELLED) return;

    string mode;

    if (ret < 6){
        char* modes[] = {"always", "game", "ps1", "xmb", "psp", "homebrew"};
        mode = modes[ret];
    }
    else if (ret == 6){
        SystemMgr::pauseDraw();
        OSK osk;
        osk.init("Game ID (i.e. ULUS01234)", "", 50);
        osk.loop();
        int osk_res = osk.getResult();
        if(osk_res != OSK_CANCEL)
        {
            char tmpText[51];
            osk.getText((char*)tmpText);
            mode = tmpText;
        }
        osk.end();
        SystemMgr::resumeDraw();
        if (osk_res == OSK_CANCEL) return;
    }
    else if (ret == 7){
        const char* path = e->getPath().c_str();
        int uid = kuKernelLoadModule(path, 0, NULL);
        int res = sceKernelStartModule(uid, strlen(path) + 1, (void*)path, NULL, NULL);
        return;
    }

    char ark_path[ARK_PATH_SIZE];
    strcpy(ark_path, common::getArkConfig()->arkpath);
    strcat(ark_path, "PLUGINS.TXT");
    string plugin = e->getPath();
    char* plugins_txt = "ms0:/SEPLUGINS/PLUGINS.TXT";
    if (plugin[0] == 'e' && plugin[1] == 'f'){
        plugins_txt[0] = 'e';
        plugins_txt[1] = 'f';
    }
    t_options_entry path_entries[] = {
        {OPTIONS_CANCELLED, "Cancel"},
        {0, plugins_txt},
        {1, ark_path},
    };

    optionsmenu = new OptionsMenu("Install path", sizeof(path_entries)/sizeof(t_options_entry), path_entries);
    int pret = optionsmenu->control();
    aux = optionsmenu;
    optionsmenu = NULL;
    delete aux;

    if (pret == OPTIONS_CANCELLED) return;

    progress_desc[0] = "Installing Plugin";
    progress_desc[1] = "";
    progress_desc[2] = "";
    progress_desc[3] = "";
    progress_desc[4] = "";
    progress = 0;
    max_progress = 1;
    draw_progress = true;

    int fd = sceIoOpen(path_entries[pret+1].name, PSP_O_WRONLY|PSP_O_CREAT|PSP_O_APPEND, 0777);
    sceIoWrite(fd, "\n", 1);
    sceIoWrite(fd, mode.c_str(), mode.size());
    sceIoWrite(fd, ", ", 2);
    sceIoWrite(fd, plugin.c_str(), plugin.size());
    sceIoWrite(fd, ", on\n", 5);
    sceIoClose(fd);

    draw_progress = false;
}

void Browser::extractArchive(int type){

    string root = get()->getPath().substr(0, 5);
    string extract_to_root = string("Extract to ")+root;

    t_options_entry options_entries[] = {
        {OPTIONS_CANCELLED, "Cancel"},
        {0, "Extract here"},
        {1, (char*)(extract_to_root.c_str())},
    };

    optionsmenu = new OptionsMenu("Extract Archive to", sizeof(options_entries)/sizeof(t_options_entry), options_entries);
    int ret = optionsmenu->control();
    OptionsMenu* aux = optionsmenu;
    optionsmenu = NULL;
    delete aux;

    if (ret == OPTIONS_CANCELLED) return;

    string name = this->get()->getName();
    string dest = ((ret == 0)? this->cwd : root) + name.substr(0, name.rfind('.')) + "/";

    printf("extracting archive to: %s\n", dest.c_str());

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

    // Refresh the list of files and dirs
    SystemMgr::pauseDraw();
    this->index = 0;
    this->start = 0;
    this->clearEntries();
    this->animating = false;
    this->draw_progress = false;
    this->optionsmenu = NULL;
    SystemMgr::resumeDraw();

    if (ftp_driver != NULL && ftp_driver->isDevicePath(this->cwd)){
        SystemMgr::pauseDraw();
        bool ftp_con = ftp_driver->connect();
        SystemMgr::resumeDraw();
        if (ftp_con){
            vector<Entry*> ftp_dir = ftp_driver->listDirectory(this->cwd);
            SystemMgr::pauseDraw();
            for (int i=0; i<ftp_dir.size(); i++){
                this->entries->push_back(ftp_dir[i]);
            }
            SystemMgr::resumeDraw();
            return;
        }
        else{
            this->cwd = ROOT_DIR;
        }    
    }

    printf("opening dir: %s\n", this->cwd.c_str());

    int dir;

    refresh_retry:
    dir = sceIoDopen(this->cwd.c_str());

    if (dir < 0){ // can't open directory
        printf("can't open\n");
        if (this->cwd == ROOT_DIR) // ms0 failed
            this->cwd = GO_ROOT; // go to ef0
        else
            this->cwd = ROOT_DIR;
        goto refresh_retry;
    }

    if (cwd == ROOT_DIR || cwd == GO_ROOT){
        devsize = common::beautifySize(common::deviceSize(cwd));
    }
    else devsize = "";

    SceIoDirent* dit = (SceIoDirent*)malloc(sizeof(SceIoDirent));

    vector<Entry*> folders;
    vector<Entry*> files;

    pspMsPrivateDirent *pri_dirent = (pspMsPrivateDirent*)malloc(sizeof(pspMsPrivateDirent));
    pri_dirent->size = sizeof(pspMsPrivateDirent);
    dit->d_private = (void*)pri_dirent;

    while ((sceIoDread(dir, dit)) > 0){
        printf("got entry: %s\n", dit->d_name);
        string ptmp = string(this->cwd)+string(dit->d_name);
        bool folder_exists = (strcmp(dit->d_name, ".") == 0 || strcmp(dit->d_name, "..") == 0 || common::folderExists(ptmp+"/"));
        if (folder_exists || FIO_SO_ISDIR(dit->d_stat.st_attr)){
            printf("is dir\n");
            if (!folder_exists){
                ptmp = string(this->cwd) + string(dit->d_name).substr(0, 4) + string(pri_dirent->s_name) + "/";
                printf("%d: %s\n", (int)common::folderExists(ptmp), ptmp.c_str());
            }
            folders.push_back(new Folder(ptmp+"/"));
        }
        else{
            printf("is file\n");
            if (!common::fileExists(ptmp)){
                ptmp = string(this->cwd) + string(dit->d_name).substr(0, 4) + string(pri_dirent->s_name);
                printf("%d: %s\n", (int)common::fileExists(ptmp), ptmp.c_str());
            }
            files.push_back(new File(ptmp));
        }
            
    }
    printf("closing and cleaning\n");
    sceIoDclose(dir);

    free(pri_dirent);
    free(dit);

    Entry* dot = NULL;
    Entry* dotdot = NULL;
    if (folders.size() > 0){
        if (folders[0]->getName() == "./"){
            dot = folders[0];
            folders.erase(folders.begin());
        }
        if (folders[0]->getName() == "../"){
            dotdot = folders[0];
            folders.erase(folders.begin());
        }
    }

    if (common::getConf()->sort_entries){
        printf("sorting entries\n");
        std::sort(folders.begin(), folders.end(), Entry::cmpEntriesForSort);
        std::sort(files.begin(), files.end(), Entry::cmpEntriesForSort);
    }

    if (!dotdot && !isRootDir(this->cwd)) dotdot = new Folder(this->cwd+"../");
    if (dotdot) folders.insert(folders.begin(), dotdot);
    if (!dot && !isRootDir(this->cwd)) dot = new Folder(this->cwd+"./");
    if (dot) folders.insert(folders.begin(), dot);
    
    printf("merging entries\n");
    SystemMgr::pauseDraw();
    for (int i=0; i<folders.size(); i++)
        entries->push_back(folders.at(i));
    for (int i=0; i<files.size(); i++)
        entries->push_back(files.at(i));
    if (this->entries->size() == 0)
        this->entries->push_back(new Folder("./"));
    SystemMgr::resumeDraw();
    
    printf("done\n");
}
        

void Browser::drawScreen(){

    const int xoffset = 115;
    int yoffset = 50;
    bool focused = (optionsmenu==NULL);
    
    // draw scrollbar (if moving)
    if (moving && entries->size() > 0){
        int height = 230/entries->size();
        int x = xoffset-65;
        int y = yoffset-13;
        ya2d_draw_rect(x+2, y, 3, height*entries->size(), DARKGRAY, 1);
        ya2d_draw_rect(x+1, y + index*height, 5, height, DARKGRAY, 1);
        ya2d_draw_rect(x+3, y, 1, height*entries->size(), LITEGRAY, 1);
        ya2d_draw_rect(x+2, y + index*height, 3, height, LITEGRAY, 1);
    }

    // draw main window
    common::getImage(IMAGE_DIALOG)->draw_scale(xoffset-50, yoffset-20, 410, 230);
    
    // no items loaded? draw wait icon
    if (entries->size() == 0){
        static float angle = 1.0;
        Image* img = common::getImage(IMAGE_WAITICON);
        img->draw_rotate((480-img->getTexture()->width)/2, (272-img->getTexture()->height)/2, angle);
        angle+=0.2;
        return;
    }
    
    // draw each entry
    for (int i=this->start; i<min(this->start+PAGE_SIZE, (int)entries->size()); i++){
        File* e = (File*)this->entries->at(i);
        // draw checkbox
        common::getCheckbox((int)e->isSelected())->draw(xoffset-30, yoffset-10);
        // draw focused entry
        if (i == index && this->enableSelection){
            if (animating){
                common::printText(xoffset, yoffset, e->getName().c_str(), LITEGRAY, SIZE_MEDIUM, focused, focused);
                animating = false;
            }
            else
                common::printText(xoffset, yoffset, e->getName().c_str(), LITEGRAY, SIZE_BIG, focused, focused);
        }
        // draw non-focused entry
        else{
            common::printText(xoffset, yoffset, this->formatText(e->getName()).c_str());
        }
        // draw entry size and icon
        common::printText(400, yoffset, e->getSize().c_str());
        common::getIcon(e->getFileType())->draw(xoffset-15, yoffset-10);
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
    static int x, y, w, h;
    switch (animation){
    case -1:
        if (w < 350 || h < 150){
            
            w += 50;
            if (w > 350)
                w = 350;
            
            h += 30;
            if (h > 150)
                h = 150;

            x = (480-w)/2;
            y = (272-h)/2;

            common::getImage(IMAGE_DIALOG)->draw_scale(x, y, w, h);
        }
        else {
            animation = 0;
            common::getImage(IMAGE_DIALOG)->draw_scale(x, y, w, h);
        }
        break;
    case 0:
        if (!this->hide_main_window){
            this->drawScreen();
            this->drawOptionsMenu();
        }
        this->drawProgress();
        if (this->optionsmenu != NULL)
            this->optionsmenu->draw();
        break;
    case 1:
        if (w > 0 || h > 0){
        
            w -= 50;
            if (w < 0)
                w = 0;
            
            h -= 30;
            if (h < 0)
                h = 0;
            
            x = (480-w)/2;
            y = (272-h)/2;
            
            common::getImage(IMAGE_DIALOG)->draw_scale(x, y, w, h);
        }
        else {
            animation = -2;
        }
        break;
    default: break;
    }
}

string Browser::formatText(string text){
    // Format the text shown, text with more than 13 characters will be truncated and ... be appended to the name
    if (text.length() <= 40)
        return text;
    else{
        string* ret = new string(text.substr(0, 37));
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

void Browser::left() {
	if (this->entries->size() == 2) return;
	if (this->index == 0) return;
	if (this->index > 0) {
		this->index = 1 * (this->index - PAGE_SIZE);
		this->start = 1 * (this->start - PAGE_SIZE);
	}
	if(this->index < 0 || this->start < 0) {
		this->index = 0;
		this->start = 0;
	}
    this->animating = true;
    common::playMenuSound();
}

void Browser::right() {
	if (this->entries->size() == 2) return;


	if (this->index + PAGE_SIZE >= entries->size()) {
        this->index = (entries->size()-1)-PAGE_SIZE+1;
		this->start = this->start-PAGE_SIZE+1;
		if(this->index > this->start+PAGE_SIZE)
			this->start = this->start+PAGE_SIZE;
	}

	   	//return;
	if (this->index == 0) {
		this->index = PAGE_SIZE-1;
		this->start = PAGE_SIZE-1;
	}
	else if (this->index < PAGE_SIZE-1){
            this->index = this->index + PAGE_SIZE-1;
			this->start = this->start + PAGE_SIZE-1;
    }
	else {
		this->index = this->index + PAGE_SIZE-1;
		this->start = this->start + PAGE_SIZE-1;
	}
	
    this->animating = true;
    common::playMenuSound();



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

        pspMsPrivateDirent *pri_dirent = (pspMsPrivateDirent*)malloc(sizeof(pspMsPrivateDirent));
        pri_dirent->size = sizeof(pspMsPrivateDirent);
        entry.d_private = (void*)pri_dirent;
        
        //allocate memory to store the full file paths
        string new_path;

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
            new_path = path + string(entry.d_name);

            if (FIO_SO_ISDIR(entry.d_stat.st_attr)){
                new_path = new_path + "/";
                if (!common::folderExists(new_path)){
                    new_path = path + string(entry.d_name).substr(0, 4) + string(pri_dirent->s_name) + "/";
                    printf("%d: %s\n", (int)common::folderExists(new_path), new_path.c_str());
                }
                recursiveFolderDelete(new_path);
            }
            else{
                if (!common::fileExists(new_path)){
                    new_path = path + string(entry.d_name).substr(0, 4) + string(pri_dirent->s_name);
                    printf("%d: %s\n", (int)common::fileExists(new_path), new_path.c_str());
                }
                self->deleteFile(new_path);
            }
            
        };
        
        sceIoDclose(d); //close directory
        sceIoRmdir(path.substr(0, path.length()-1).c_str()); //delete empty folder
        free(pri_dirent);
    };
}

long Browser::recursiveSize(string path){
    SceUID d = sceIoDopen(path.c_str());
    
    long total_size = 0;

    if(d >= 0)
    {
        SceIoDirent entry;
        memset(&entry, 0, sizeof(SceIoDirent));

        pspMsPrivateDirent *pri_dirent = (pspMsPrivateDirent*)malloc(sizeof(pspMsPrivateDirent));
        pri_dirent->size = sizeof(pspMsPrivateDirent);
        entry.d_private = (void*)pri_dirent;
        
        //allocate memory to store the full file paths
        string new_path;

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
            new_path = path + string(entry.d_name);

            if (FIO_SO_ISDIR(entry.d_stat.st_attr)){
                new_path = new_path + "/";
                if (!common::folderExists(new_path)){
                    new_path = path + string(entry.d_name).substr(0, 4) + string(pri_dirent->s_name) + "/";
                    printf("%d: %s\n", (int)common::folderExists(new_path), new_path.c_str());
                }
                total_size += recursiveSize(new_path);
            }
            else{
                if (!common::fileExists(new_path)){
                    new_path = path + string(entry.d_name).substr(0, 4) + string(pri_dirent->s_name);
                    printf("%d: %s\n", (int)common::fileExists(new_path), new_path.c_str());
                }
                total_size += common::fileSize(new_path);
            }
            
        };
        
        sceIoDclose(d); //close directory
        sceIoRmdir(path.substr(0, path.length()-1).c_str()); //delete empty folder
        free(pri_dirent);
    }
    else if ((d=sceIoOpen(path.c_str(), PSP_O_RDONLY, 0777)) >= 0){
        total_size = sceIoLseek(d, 0, SEEK_END);
        sceIoClose(d);
    }

    return total_size;
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

    if (ftp_driver != NULL && ftp_driver->isDevicePath(path)){
        ftp_driver->deleteFolder(path);
    }
    else {
        recursiveFolderDelete(path);
    }
    
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
    
    if (ftp_driver != NULL && ftp_driver->isDevicePath(path)){
        ftp_driver->deleteFile(path);
    }
    else{
        sceIoRemove(path.c_str());
    }
    
    if (!noRedraw)
        draw_progress = false;
}

int Browser::pspIoMove(string src, string dest)
{

    if ( *(u32*)(src.c_str()) != *(u32*)(dest.c_str()) )
        return -1; // not in the same device

    progress_desc[0] = "Moving file/folder";
    progress_desc[1] = "    "+src;
    progress_desc[2] = "into";
    progress_desc[3] = "    "+dest;
    progress_desc[4] = "";
    progress = 0;
    max_progress = 100;
    
    bool noRedraw = draw_progress;
    if (!noRedraw)
        draw_progress = true;

    const char deviceSize = 4;

    if (dest[dest.length()-1] != '/') dest += "/";
    size_t lastSlash = src.rfind("/", string::npos);
    string name = src.substr(lastSlash+1, string::npos);
    string new_dest = dest+name;

    if (src[src.length()-1] == '/'){
        src = src.substr(0, src.length()-1);
        new_dest = new_dest.substr(0, new_dest.length()-1);
    }

    u32 data[2];
    data[0] = (u32)src.c_str() + deviceSize;
    data[1] = (u32)new_dest.c_str() + deviceSize;

    int res = sceIoDevctl((*(u32*)(src.c_str()) == EF0_PATH)?"ef0:":"ms0:", 0x02415830, data, sizeof(data), NULL, 0);

    if (!noRedraw)
        draw_progress = false;

    return res;
}

int Browser::copy_folder_recursive(const char * source, const char * destination)
{

    draw_progress = true;

    printf("Source: %s\n", source);
    printf("Destination: %s\n", destination);

    //create new folder
    if (ftp_driver != NULL && ftp_driver->isDevicePath(destination)){
        string ftp_path = string(destination);
        ftp_driver->createFolder(ftp_path);
    }
    else sceIoMkdir(destination, 0777);
    
    string new_destination = destination;
    if (new_destination[new_destination.length()-1] != '/') new_destination += "/";
    string new_source = source;
    if (new_source[new_source.length()-1] != '/') new_source += "/";
    
    if (ftp_driver != NULL && ftp_driver->isDevicePath(source)){
        vector<Entry*> entries = ftp_driver->listDirectory(new_source);
        for (int i=0; i<entries.size(); i++){
            Entry* e = entries[i];
            entries[i] = NULL;
            printf("Copying %s\n", e->getName().c_str());
            if (e->getName() != "<refresh>" && e->getName() != "<disconnect>" && e->getName() != "./" && e->getName() != "../"){
                string src = new_source + e->getName();
                if (e->getType() == "FOLDER"){
                    string dst = new_destination + e->getName().substr(0, e->getName().length()-1);
                    copy_folder_recursive(src.c_str(), dst.c_str());
                }
                else{
                    copyFile(src, new_destination); //copy file
                }
            }
            delete e;
        }
    }
    else{
        //try to open source folder
        SceUID dir = sceIoDopen(source);
        
        if(dir >= 0)
        {
            SceIoDirent entry;
            memset(&entry, 0, sizeof(SceIoDirent));

            pspMsPrivateDirent* pri_dirent = (pspMsPrivateDirent*)malloc(sizeof(pspMsPrivateDirent));
            pri_dirent->size = sizeof(pspMsPrivateDirent);
            entry.d_private = (void*)pri_dirent;
            
            //start reading directory entries
            while(sceIoDread(dir, &entry) > 0)
            {
                //skip . and .. entries
                if (!strcmp(".", entry.d_name) || !strcmp("..", entry.d_name)) 
                {
                    memset(&entry, 0, sizeof(SceIoDirent));
                    continue;
                };
                string src = new_source + entry.d_name;

                if (FIO_SO_ISDIR(entry.d_stat.st_attr)){
                    string dst;
                    if (!common::folderExists(src)){
                        string sname = string(entry.d_name).substr(0, 4) + string(pri_dirent->s_name);
                        src = new_source + sname;
                        dst = new_destination + sname;
                        printf("%d: %s\n", (int)common::folderExists(src), src.c_str());
                    }
                    else{
                        dst = new_destination + entry.d_name;
                    }
                    copy_folder_recursive(src.c_str(), dst.c_str());
                }
                else{
                    if (!common::fileExists(src)){
                        src = new_source + string(entry.d_name).substr(0, 4) + string(pri_dirent->s_name);
                        printf("%d: %s\n", (int)common::fileExists(src), src.c_str());
                    }
                    if (pasteMode == COPY || (pasteMode == CUT && pspIoMove(src, new_destination) < 0))
                        copyFile(src, new_destination); //copy file
                }

            };
            //close folder
            sceIoDclose(dir);
            free(pri_dirent);
        };
    }
    
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
    
    progress_desc[0] = "Copying file";
    progress_desc[1] = "    "+path;
    progress_desc[2] = "into";
    progress_desc[3] = "    "+dest;
    progress_desc[4] = "";
    progress = 0;
    max_progress = 100;
    
    bool noRedraw = draw_progress;
    if (!noRedraw)
        draw_progress = true;
    
    printf("source file: %s\n", path.c_str());
    printf("destination: %s\n", destination.c_str());
    
    if (ftp_driver != NULL && ftp_driver->isDevicePath(path)){
        // download from FTP
        ftp_driver->copyFileFrom(path, destination, &progress);
    }
    else if (ftp_driver != NULL && ftp_driver->isDevicePath(destination)){
        // upload to FTP
        ftp_driver->copyFileTo(path, destination, &progress);
    }
    else{
        // local copy
        SceUID src = sceIoOpen(path.c_str(), PSP_O_RDONLY, 0777);
        SceUID dst = sceIoOpen(dest.c_str(), PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
        progress = 0;
        max_progress = sceIoLseek(src, 0, SEEK_END);
        sceIoLseek(src, 0, SEEK_SET);

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
    }
    
    if (!noRedraw)
        draw_progress = false;
}

void Browser::copyFile(string path){
    copyFile(path, this->cwd);
}

void Browser::fillClipboard(){
    this->clipboard->clear();
    for (int i=0; i<entries->size(); i++){
        BrowserFile* e = (File*)entries->at(i);
        if (e->isSelected())
            this->clipboard->push_back(e->getPath());
    }
}

void Browser::copy(){
    // Mark the paste mode as copy
    this->pasteMode = COPY;
    this->fillClipboard();
}

void Browser::cut(){
    // Mark the paste mode as cut
    this->pasteMode = CUT;
    this->fillClipboard();
}

void Browser::paste(){
    // Copy or cut all paths in the paste buffer to the cwd
    printf("paste command\n");
    for (int i = 0; i<clipboard->size(); i++){
        string path = clipboard->at(i);
        printf("pasting %s\n", path.c_str());
        if (path[path.length()-1] == '/'){
            if (pasteMode == CUT){
                if (pspIoMove(path, this->cwd) < 0){
                    this->copyFolder(path);
                    this->deleteFolder(path);
                }
            }
            else{
                printf("copy folder\n");
                this->copyFolder(path);
            }
        }
        else{
            if (pasteMode == CUT){
                printf("move file\n");
                if (pspIoMove(path, this->cwd) < 0){
                    this->copyFile(path);
                    this->deleteFile(path);
                }
            }
            else{
                printf("copy file\n");
                this->copyFile(path);
            }
        }
    }
    if (clipboard->size()){
        this->refreshDirs();
        GameManager::updateGameList(cwd.c_str()); // tell GameManager to update
    }
    this->clipboard->clear();
}

void Browser::rename(){
    if (ftp_driver != NULL && ftp_driver->isDevicePath(this->get()->getPath())){
        return; // can't rename remote files
    }
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
    }
    osk.end();
    free(oldname);
    SystemMgr::resumeDraw();
    this->refreshDirs();
    GameManager::updateGameList(cwd.c_str()); // tell GameManager to update
}

void Browser::removeSelection(){
    // Delete all paths in the paste buffer

    t_options_entry opts[] = {
        {OPTIONS_CANCELLED, "Cancel"},
        {0, "Confirm"},
    };

    optionsmenu = new OptionsMenu("Confirm Deletion?", sizeof(opts)/sizeof(t_options_entry), opts);
    int pret = optionsmenu->control();
    OptionsMenu* aux = optionsmenu;
    optionsmenu = NULL;
    delete aux;

    if (pret == OPTIONS_CANCELLED) return;

    draw_progress = true;
    this->fillClipboard();
    if (this->clipboard->size() == 0)
        this->clipboard->push_back(this->get()->getPath());
        
    if (this->clipboard->size() > 0){
        for (int i = 0; i<clipboard->size(); i++){
            string path = clipboard->at(i);
            if (path[path.length()-1] == '/'){
                deleteFolder(path);
            }
            else{
                deleteFile(path);
            }
        }
        this->clipboard->clear();
        this->refreshDirs();
        GameManager::updateGameList(cwd.c_str()); // tell GameManager to update
    }
    draw_progress = false;
}

void Browser::makedir(){
    SystemMgr::pauseDraw();
    OSK osk;
    osk.init("Name of new folder", "new folder", 50);
    osk.loop();
    if(osk.getResult() != OSK_CANCEL)
    {
        char tmpText[51];
        osk.getText((char*)tmpText);
        string dirName = string(tmpText);
        if (ftp_driver != NULL && ftp_driver->isDevicePath(this->cwd)){
            ftp_driver->createFolder(dirName);
        }
        else sceIoMkdir((this->cwd+dirName).c_str(), 0777);
    }
    osk.end();
    SystemMgr::resumeDraw();
    this->refreshDirs();
}

void Browser::makefile(){
    SystemMgr::pauseDraw();
    OSK osk;
    osk.init("Name of new file", "new file", 50);
    osk.loop();
    if(osk.getResult() != OSK_CANCEL)
    {
        char tmpText[51];
        osk.getText((char*)tmpText);
        string fileName = string(tmpText);
        if (ftp_driver != NULL && ftp_driver->isDevicePath(this->cwd)){
            ftp_driver->createFile(fileName);
        }
        else sceIoOpen((this->cwd+fileName).c_str(), PSP_O_WRONLY | PSP_O_CREAT, 0777);
    }
    osk.end();
    SystemMgr::resumeDraw();
    this->refreshDirs();
}

void Browser::createNew(){
    t_options_entry opts[] = {
        {OPTIONS_CANCELLED, "Cancel"},
        {0, "Folder"},
        {1, "File"},
    };

    optionsmenu = new OptionsMenu("Create new...", sizeof(opts)/sizeof(t_options_entry), opts);
    int ret = optionsmenu->control();
    OptionsMenu* aux = optionsmenu;
    optionsmenu = NULL;
    delete aux;

    switch (ret){
        case 0: makedir(); break;
        case 1: makefile(); break;
    }
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
        
            {
            int x = 10;
            int y = 80;
            for (int i=0; i<MAX_OPTIONS; i++){
                if (pEntries[i] == NULL) continue;
                if (i == pEntryIndex)
                    common::printText(x, y, pEntries[i], LITEGRAY, SIZE_BIG, true);
                else
                    common::printText(x, y, pEntries[i]);
                y += 20;
            }
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

    pad->update();
    
    while (true){
        
        pad->update();
       // Down 
        if (pad->down()){
            common::playMenuSound();
            do {
                if (pEntryIndex < MAX_OPTIONS-1){
                    pEntryIndex++;
                }
                else{
                    pEntryIndex = 0;
                }
            } while (pEntries[pEntryIndex] == NULL);
        }
		// Up
        else if (pad->up()){
            common::playMenuSound();
            do {
                if (pEntryIndex > 0){
                    pEntryIndex--;
                }
                else{
                    pEntryIndex = MAX_OPTIONS-1;
                }
            } while (pEntries[pEntryIndex] == NULL);
        }
		// Right
		else if (pad->right()) {
			common::playMenuSound();
			do {
				if (pEntryIndex+3 < MAX_OPTIONS-1) {
					pEntryIndex += 3;
				}
				else {
					pEntryIndex = MAX_OPTIONS-1;
				}
			} while (pEntries[pEntryIndex] == NULL);
		}
		// Left
		else if (pad->left()) {
			common::playMenuSound();
			do {
				if (pEntryIndex-3 < 0)
					pEntryIndex = 0;
				else
					pEntryIndex -= 3;
			} while (pEntries[pEntryIndex] == NULL);
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
    case NO_MODE:                                                      break;
    case COPY:        this->copy();                                    break;
    case CUT:         this->cut();                                     break;
    case PASTE:       this->paste();                                   break;
    case DELETE:      this->removeSelection();                         break;
    case RENAME:      this->rename();                                  break;
    case CREATE:      this->createNew();                               break;
    case MS0_DIR:     this->cwd = ROOT_DIR;     this->refreshDirs();   break;
    case FTP_DIR:     this->cwd = FTP_ROOT;     this->refreshDirs();   break;
    case EF0_DIR:     this->cwd = GO_ROOT;      this->refreshDirs();   break;
    case UMD_DIR:     this->cwd = UMD_ROOT;     this->refreshDirs();   break;
    }
}
        
void Browser::control(Controller* pad){
    // Control the menu through user input
    if (pad->up())
        this->up();
    else if (pad->down())
        this->down();
	else if (pad->right())
		this->right();
	else if (pad->left())
		this->left();
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
        common::playMenuSound();
        this->refreshDirs();
    }
    else{
        if (moving) moving--;
    }
}
