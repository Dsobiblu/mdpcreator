#include "MDPWindow.h"
#include <cstdlib>
#include <QTime>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QIcon>
#include <QtCore/qmath.h>
#include <QTextStream>
#include <QFile>
#include <QMenuBar>
#include <QMessageBox>

MDPWindow::MDPWindow(QWidget* parent) : QMainWindow(parent)
{
    QTime time(QTime::currentTime());
    qsrand(time.msec());

    initialiseCharLists();
    MAX_FRENCH = countNumberOfWords(":/french_dict.txt");
    MAX_ENGLISH = countNumberOfWords(":/english_dict.txt");

    this->setWindowTitle("MDPCreator 2.1");
    this->setWindowIcon(QIcon(":/icon.jpg"));
    this->setMinimumSize(800,250);
    QWidget* widget = new QWidget;
    this->setCentralWidget(widget);
    initialiseMenuBar();

    // drop-down list: 'level of security'
    QVBoxLayout* securityLayout = new QVBoxLayout;
    mdpSecurityLabel = new QLabel("Niveau de sécurité :", this);
    mdpSecurity = new QComboBox(this);
    QIcon lowIcon(":/low.png");
    QIcon midelIcon(":/mid.png");
    QIcon highIcon(":/high.png");
    QIcon veryHighIcon(":/vhigh.png");
    QIcon veryHighEasyIcon(":/vhigheasy.png");
    mdpSecurity->addItem(lowIcon, "Normal");
    mdpSecurity->addItem(midelIcon, "Moyen");
    mdpSecurity->addItem(highIcon, "Élevé");
    mdpSecurity->addItem(veryHighIcon, "Très élevé");
    mdpSecurity->addItem(veryHighEasyIcon, "Très élevé ET facile à retenir");
    mdpSecurity->setIconSize(QSize(48,48));
    securityLayout->addWidget(mdpSecurityLabel);
    securityLayout->addWidget(mdpSecurity);

    // spin box: length of password
    QVBoxLayout* lengthLayout = new QVBoxLayout;
    mdpLengthLabel = new QLabel("Longueur du mot de passe :", this);
    mdpLength = new QSpinBox(this);
    lengthLayout->addWidget(mdpLengthLabel);
    lengthLayout->addWidget(mdpLength);

    // if password with words: choose French or English dictionary
    mdpLanguage = new QGroupBox("Langue");
    chooseFrench  = new QRadioButton("Français");
    chooseEnglish = new QRadioButton("Anglais");
    chooseFrench->setChecked(true);
    QVBoxLayout *languageLayout = new QVBoxLayout;
    languageLayout->addWidget(chooseFrench);
    languageLayout->addWidget(chooseEnglish);
    mdpLanguage->setLayout(languageLayout);

    // button + text field: generate and see password
    generateMdpButton = new QPushButton("Générer !");
    generateMdpButton->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Expanding);
    changeMode(0);

    QVBoxLayout* resultingMdpLayout = new QVBoxLayout;
    resultingMdp = new QLabel("Mot de passe généré :", this);
    resultingMdpEdit = new QLineEdit(this);
    resultingMdpEdit->setReadOnly(true);
    resultingMdpLayout->addWidget(resultingMdp);
    resultingMdpLayout->addWidget(resultingMdpEdit);

    QGridLayout* generalLayout = new QGridLayout;
    generalLayout->addLayout(securityLayout,0,0,1,2);
    generalLayout->addLayout(lengthLayout,1,0,1,1,Qt::AlignTop);
    generalLayout->addWidget(mdpLanguage,1,1,1,1);
    generalLayout->addWidget(generateMdpButton,0,2,2,1);
    generalLayout->addLayout(resultingMdpLayout,2,0,1,3);
    widget->setLayout(generalLayout);

    connect(generateMdpButton,SIGNAL(clicked()),this,SLOT(generateMdp()));
    connect(mdpSecurity,SIGNAL(currentIndexChanged(int)),this,SLOT(changeMode(int)));
}

void MDPWindow::generateMdp()
{
    QString mdp;
    // password made of words
    if(mdpSecurity->currentIndex() == 4){
        QFile dict;
        if(chooseFrench->isChecked()){
            dict.setFileName(":/french_dict.txt");
        }else{
            dict.setFileName(":/english_dict.txt");
        }
        if(dict.open(QFile::ReadOnly)){
            QTextStream in(&dict);
            QVector<int> lines(mdpLength->value());
            for(int i=0; i<lines.size(); i++){
                double unif = qrand()/double(RAND_MAX);
                if(chooseFrench->isChecked()){
                    lines[i] = qCeil(unif*MAX_FRENCH);
                }else{
                    lines[i] = qCeil(unif*MAX_ENGLISH);
                }
            }
            QVector<int> lines_ord(lines);
            std::sort(lines_ord.begin(),lines_ord.end());//avoid multiple look-ups in database
            QStringList mots;
            int counter=0;
            for(int i=0; i<lines_ord.size(); i++){
                while(++counter<lines_ord[i] and !in.atEnd()){
                    in.readLine();
                }
                mots << in.readLine();
            }
            for(int i=0; i<lines.size(); i++){
                mdp += " - " + mots[lines_ord.indexOf(lines[i])];
            }
            mdp.remove(0,3);
            dict.close();
        }
    // password made of random characters
    }else{
        for(int i=0 ; i<mdpLength->value() ; ++i){
            int nb_classes  = mdpSecurity->currentIndex()+1;
            int which_class = qrand() % nb_classes;
            int nb_within = charLists[which_class].size();
            int which_within = qrand() % nb_within;
            mdp.append(charLists[which_class][which_within]);
        }
    }
    resultingMdpEdit->setText(mdp);
}

void MDPWindow::changeMode(int index)
{
    if(index < 4){
        mdpLength->setSuffix(" caractères");
        mdpLength->setRange(5,35);
        mdpLength->setValue(7);
        mdpLanguage->setDisabled(true);
        if(index == 0){
            generateMdpButton->setStyleSheet("QPushButton{ background-image: url(:/abc.png);"
                                             "background-position: center center;"
                                             "background-repeat: no-repeat; }");
        }else if(index == 1){
            generateMdpButton->setStyleSheet("QPushButton{ background-image: url(:/aBc.png);"
                                             "background-position: center center;"
                                             "background-repeat: no-repeat; }");
        }else if(index == 2){
            generateMdpButton->setStyleSheet("QPushButton{ background-image: url(:/aB6.png);"
                                             "background-position: center center;"
                                             "background-repeat: no-repeat; }");
        }else{
            generateMdpButton->setStyleSheet("QPushButton{ background-image: url(:/ad6.png);"
                                             "background-position: center center;"
                                             "background-repeat: no-repeat; }");
        }
    }else{
        mdpLength->setSuffix(" mots");
        mdpLength->setRange(3,8);
        mdpLength->setValue(3);
        mdpLanguage->setEnabled(true);
        generateMdpButton->setStyleSheet("QPushButton{ background-image: url(:/mot.png);"
                                         "background-position: center center;"
                                         "background-repeat: no-repeat; }");
    }
}

void MDPWindow::initialiseCharLists()
{
    QStringList chars;
    chars <<"a"<<"b"<<"c"<<"d"<<"e"<<"f"<<"g"<<"h"<<"i"<<"j"<<"k"<<"l"<<"m"<<"n"<<"o"<<"p"<<"q"<<"r"<<"s"<<"t"<<"u"<<"v"<<"w"<<"x"<<"y"<<"z";
    charLists.clear();
    charLists.push_back(chars);
    chars.clear();
    chars <<"A"<<"B"<<"C"<<"D"<<"E"<<"F"<<"G"<<"H"<<"I"<<"J"<<"K"<<"L"<<"M"<<"N"<<"O"<<"P"<<"Q"<<"R"<<"S"<<"T"<<"U"<<"V"<<"W"<<"X"<<"Y"<<"Z";
    charLists.push_back(chars);
    chars.clear();
    chars <<"0"<<"1"<<"2"<<"3"<<"4"<<"5"<<"6"<<"7"<<"8"<<"9";
    charLists.push_back(chars);
    chars.clear();
    chars <<"°"<<"+"<<"*"<<"_"<<"%"<<"&"<<"/"<<"("<<")"<<"="<<"?"<<"!"<<""<<"@"<<"#"<<"<"<<">"<<","<<";"<<"."<<":"<<"-";
    charLists.push_back(chars);
}

unsigned int MDPWindow::countNumberOfWords(const QString &file) const{
    QFile ff(file);
    unsigned int counter=0;
    if(ff.open(QFile::ReadOnly)){
        QTextStream in(&ff);
        while(!in.atEnd()){
            in.readLine();
            counter++;
        }
    }
    ff.close();
    return counter;
}

void MDPWindow::initialiseMenuBar(){
    aboutMDPCreatorAct = new QAction("À propos de &MPDCreator...", this);
    aboutQtAct         = new QAction("À propos de &Qt...", this);
    connect(aboutMDPCreatorAct, SIGNAL(triggered()), this, SLOT(aboutMDPCreator()));
    connect(aboutQtAct, SIGNAL(triggered()), this, SLOT(aboutQt()));
    // build menu
    aboutMenu = menuBar()->addMenu("À propos");
    aboutMenu->addAction(aboutQtAct);
    aboutMenu->addAction(aboutMDPCreatorAct);
}

void MDPWindow::aboutMDPCreator(){
    QString title = "MDPCreator version 2.1";
    QString text  = "<strong>MDPCreator version 2.1</strong><br/><br/>"
            "Imaginé et créé par Thomas Lugrin © 2014-2017 GPL-2+<br/><br/>"
            "Les figures noir et blanc proviennent de <em>Xinh Studio</em> et sont sous la licence<br/>"
            "<a href='http://creativecommons.org/licenses/by/3.0/'>Creative Commons Attribution 3.0 Unported Licence</a>.<br/><br/>"
            "Les bases de données de mots français et anglais proviennent de"
            "<ul><li><a href='http://www.lexique.org'>Lexique 3.82</a> et</li>"
            "<li><a href='http://dreamsteep.com/projects/the-english-open-word-list.html'>English Open Word List</a> "
            "basé sur le <a href='http://www.crosswordman.com/wordlist.html'>UK Advanced Cryptics Dictionary</li></ul>"
            "et sont respectivement sous la licence <a href='http://creativecommons.org/licenses/by/3.0/'>Creative Commons Attribution 3.0 Unported Licence</a> "
            "et une licence de logiciel libre disponible dans les sources et <a href='http://dreamsteep.com/projects/the-english-open-word-list.html'>en ligne</a>.";
    QMessageBox msgBox(this);
    msgBox.setWindowTitle(title);
    msgBox.setText(text);
    msgBox.setIconPixmap(QPixmap(":/icon.ico"));
    QSpacerItem* horizontalSpacer = new QSpacerItem(1000, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
    QGridLayout* layout = (QGridLayout*)msgBox.layout();
    layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());
    msgBox.exec();
}

void MDPWindow::aboutQt(){
    QMessageBox::aboutQt(this);
}
