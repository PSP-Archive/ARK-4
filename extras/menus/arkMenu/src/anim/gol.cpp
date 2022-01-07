#include "gol.h"
#include "common.h"

GoLAnim::GoLAnim(){
    generateRandom();
}

GoLAnim::~GoLAnim(){

}

void GoLAnim::generateRandom(){
    generation = 0;
    for (int i=0; i<GOL_ROW; i++){
        for (int j=0; j<GOL_COL; j++){
            a[i][j] = ((unsigned char)rand())%2;
        }
    }
}

//returns the count of alive neighbours
int GoLAnim::count_live_neighbour_cell(int r, int c){
    int i, j, count=0;
    for(i=r-1; i<=r+1; i++){
        for(j=c-1;j<=c+1;j++){
            if((i==r && j==c) || (i<0 || j<0) || (i>=GOL_ROW || j>=GOL_COL)){
                continue;
            }
            if(a[i][j]==1){
                count++;
            }
        }
    }
    return count;
}
        

void GoLAnim::draw(){
    //next canvas values based on live neighbour count
    int i,j;
    memset(b, 0, sizeof(b));
    generation++;
    for(i=0; i<GOL_ROW; i++){
        for(j=0;j<GOL_COL;j++){
            int neighbour_live_cell = count_live_neighbour_cell(i,j);
            if(a[i][j]==1 && (neighbour_live_cell==2 || neighbour_live_cell==3)){
                b[i][j]=1;
            }
 
            else if(a[i][j]==0 && neighbour_live_cell==3){
                b[i][j]=1;
            }
            else{
                b[i][j]=0;
            }
            // draw
            if (b[i][j]) common::getImage(IMAGE_SPRITE)->draw_scale((j+1)*GOL_W, (i+1)*GOL_H, (int)GOL_W, (int)GOL_H);
        
        }
    }
    if (generation > MAX_GENERATIONS) generateRandom();
    else memcpy(a, b, sizeof(b));
}
