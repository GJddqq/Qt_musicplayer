#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>
#include <QMediaPlaylist>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QList>
#include <QMimeDatabase>
#include <QCoreApplication>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    initplayer();
}

MainWindow::~MainWindow()
{
    delete ui;
}

//界面ui未完成的在这里设计ui和逻辑初始化设置
void MainWindow::initplayer()
{
    ui->pushButton_4->setEnabled(false);    //未选择文件不让播放
    voi = new QMediaPlayer(this);
    voi->setVolume(volume);
    voilist = new QMediaPlaylist;
    voilist->setPlaybackMode(QMediaPlaylist::Loop); //默认循环播放列表中的歌曲
    voi->setPlaylist(voilist);
    this->voi->setVolume(100);          //初始化播放音量为100
    ui->horizontalSlider_2->setValue(this->voi->volume());  //初始化音量滚动轮
    connect(voi,&QMediaPlayer::positionChanged,this,&MainWindow::updatePosition);
    connect(voi,&QMediaPlayer::durationChanged,this,&MainWindow::updateDuration);
}

//实现歌曲的播放和按钮转换
void MainWindow::voiplay()
{
    voi->play();
    this->ui->pushButton_4->setToolTip("暂停");
    this->ui->pushButton_4->setIcon(QIcon(":/img/play.png"));   //图标的更换
}

//实现歌曲的暂停和按钮转换
void MainWindow::voipause()
{
    voi->pause();   //暂停，会从暂停的位置开始播放
    this->ui->pushButton_4->setToolTip("播放");
    this->ui->pushButton_4->setIcon(QIcon(":/img/pause.png"));

}

//实现歌曲的停止
void MainWindow::voistop(){
    voi->stop();
}

//播放暂停按键状态识别
void MainWindow::on_pushButton_4_clicked()
{
    //如果没有在播放
    if(voi->state() == QMediaPlayer::PlayingState)
    {
        this->voipause();
    }
    else
    {
        this->voiplay();
    }

}

//对话框选取要播放的音乐文件并将文件名添加到歌单窗口
//有的文件名识别标题不是很正确，比如“山海-草东没有排队.mp3”识别出来是"10_山海"
void MainWindow::on_pushButton_clicked()
{
    //选择的时候筛选文件，只能选择mp3、wav、mpga格式的文件
    QList<QUrl> path =  QFileDialog::getOpenFileUrls(this,tr("请选择要播放的音乐"),QUrl( "C:/"),"music(*.mp3 *.wav *.mpga)");
    qDebug()<<"path"<<path;

    QMediaPlayer tmpPlayer;//解析文件信息的临时播放器
    foreach(QUrl i,path )
    {
        voilist->addMedia(i);   //添加到播放列表
        qDebug("voi file url adding");

        tmpPlayer.setMedia(i);  //将歌名添加到歌单窗口
        while(!tmpPlayer.isMetaDataAvailable())
        {
            QCoreApplication::processEvents();
        }
        //其它信息暂时不添加
        if(tmpPlayer.isMetaDataAvailable())
        {
            ui->listWidget->addItem((tmpPlayer.metaData(QStringLiteral("Title")).toString()));
        }
    }
    //选取文件后自动播放
    if(!path.isEmpty())
    {   qDebug("voi file ready");

        if(voi->state() != QMediaPlayer::PlayingState && voi->state() != QMediaPlayer::PausedState)
        {
            voi->setPlaylist(voilist);
            this->voiplay();
            ui->pushButton_4->setEnabled(true);//恢复按钮的功能
        }

    }
    else    //没有选取音乐就提示
    {
       qDebug("voi file not choose");
       QMessageBox::warning(this,tr("温馨提示"),tr("你没有选择音乐文件\n请重新选择"));
    }

}


//鼠标滚轮上下滚动实现音量调节
void MainWindow::on_horizontalSlider_2_sliderMoved(int position)
{
    this->voi->setVolume(position);
    if(this->voi->volume() > 0){
        ui->toolButton->setIcon(QIcon(":/img/volume.png"));
        this->voi->setMuted(false);
        ui->horizontalSlider_2->setValue(this->voi->volume());
    }
}

//静音
void MainWindow::on_toolButton_clicked()
{
    ui->horizontalSlider_2->setValue(this->voi->volume());
    if(this->voi->isMuted()){
        ui->toolButton->setIcon(QIcon(":/img/volume.png"));
        this->voi->setMuted(false);
        this->voi->setVolume(100);
        ui->horizontalSlider_2->setValue(this->voi->volume());
    }
    else{
        ui->toolButton->setIcon(QIcon(":/img/mute.png"));
        this->voi->setMuted(true);
        this->voi->setVolume(0);
        ui->horizontalSlider_2->setValue(0);
    }
}

//当前文件播放位置变化，更新进度显示
void MainWindow::updatePosition(qint64 position){
    if(ui->horizontalSlider->isSliderDown())
        return;
    ui->horizontalSlider->setValue(position);
    int secs = position/1000;
    int mins = secs/60;
    secs = secs % 60;
    positionTime = QString::asprintf("%d:%d",mins,secs);
    ui->sjlabel->setText(positionTime +"/"+ durationTime);
}

//文件时长变化，更新当前播放文件名显示
void MainWindow::updateDuration(qint64 duration){
    ui->horizontalSlider->setMaximum(duration);
    int secs = duration/1000;
    int mins = secs/60;
    secs = secs % 60;
    durationTime = QString::asprintf("%d:%d",mins,secs);
    ui->sjlabel->setText(positionTime +"/"+ durationTime);
}

//根据进度条传入参数，实现拖动进度条改变歌曲进度
void MainWindow::on_horizontalSlider_sliderReleased()
{
    voi->setPosition(ui->horizontalSlider->value());
}

void MainWindow::on_lasttoolButton_clicked()
{
   int row = voilist->mediaCount(); //记录列表歌曲数量
   if(row == 1){    //当列表只有一首歌时，重新播放
       voi->stop();
       voi->play();
   }
   if(this->voilist->playbackMode() == QMediaPlaylist::CurrentItemInLoop){     //单曲循环时，切歌也是重新播放
       voi->stop();
       voi->play();
   }
   voilist->previous();
   this->ui->lasttoolButton->setToolTip("上一首");
}

void MainWindow::on_nexttoolButton_clicked()
{
    int row = voilist->mediaCount(); //记录列表歌曲数量
    if(row == 1){   //当列表只有一首歌时，重新播放
        voi->stop();
        voi->play();
    }
    if(this->voilist->playbackMode() == QMediaPlaylist::CurrentItemInLoop){     //单曲循环时，切歌也是重新播放
        voi->stop();
        voi->play();
    }
    voilist->next();
    this->ui->nexttoolButton->setToolTip("下一首");
}

void MainWindow::on_toolButton_2_clicked()
{
    if(this->voilist->playbackMode() == QMediaPlaylist::Loop){
         voilist->setPlaybackMode(QMediaPlaylist::Random);
         ui->toolButton_2->setIcon(QIcon(":/img/random.png"));
    }
    else if(this->voilist->playbackMode() == QMediaPlaylist::Random){
         voilist->setPlaybackMode(QMediaPlaylist::CurrentItemInLoop);
         ui->toolButton_2->setIcon(QIcon(":/img/currentloop.png"));
    }
    else{
         voilist->setPlaybackMode(QMediaPlaylist::Loop);
         ui->toolButton_2->setIcon(QIcon(":/img/loop.png"));
    }

}
