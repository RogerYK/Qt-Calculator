#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QDebug>
#include <map>
#include <stack>
#include <QTextBlock>
#include <QClipboard>
#include <QScrollBar>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    initUi();
    waitForOperator = false;
    connectSlots();
    setShortcutKeys();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::abortOperation()
{
    setCurrentText("0");
    waitForOperator = true;
    statusBar()->showMessage(error, 4000);
}

void MainWindow::digitClicked()
{
    QPushButton *digitBtn = static_cast<QPushButton*>(sender());
    QString value = digitBtn->text();
    QString current = currentText();
    if(current == "0" && value == "0")
        return;
    if(waitForOperator)
    {
        log.append(current + '\n');
        ui->textEdit->append(value);
        ui->textEdit->moveCursor(QTextCursor::End);
        waitForOperator = false;
    }
    else
    {
        if(current != "0")
            setCurrentText(current + value);
        else
            setCurrentText(value);
    }
}

void MainWindow::on_clearBtn_clicked()
{
    //将当前显示的数归零
    setCurrentText("0");
    waitForOperator = false;
}

void MainWindow::on_clearAllBtn_clicked()
{
    //将当前显示的数据归零，并将之前保存的数据运算清除
    ui->textEdit->setText("0");
    waitForOperator = false;
}

void MainWindow::on_equalBtn_clicked()
{
    double result = 0.0;
    try
    {
        result = compute(inToPost(currentText()));
    }
    catch(const char *er)
    {
        error = er;
        abortOperation();
        return;
    }
    log.append(currentText() + '\n');
    ui->textEdit->append(QString::number(result));
    ui->textEdit->moveCursor(QTextCursor::End);
    waitForOperator = true;
}

void MainWindow::on_signBtn_clicked()
{
    QString current = currentText();
    if (waitForOperator)
    {
        log.append(current + '\n');
        ui->textEdit->append("-");
        ui->textEdit->moveCursor(QTextCursor::End);
        waitForOperator = false;
    }
    else
    {
        if (current[0] == '-')
            current[0] = '+';
        else
            current[0] = '-';
        setCurrentText(current);
    }
}

void MainWindow::operatorClicked()
{
    QPushButton *clickedBtn = qobject_cast<QPushButton *>(sender());
    QString clickedOperator = clickedBtn->text();
    setCurrentText(currentText() + clickedOperator);
    waitForOperator = false;
}

void MainWindow::on_pointBtn_clicked()
{
    setCurrentText(currentText() + ".");
    waitForOperator = false;
}

QString MainWindow::inToPost(QString infix) throw(const char*)
{
    std::stack<char> stack;
    char current = 0;//读入的字符
    QString postfix;//写入后缀表达式的字符串

    std::map<char,int> priority;//运算符号优先级表
    priority['+'] = 0;
    priority['-'] = 0;
    priority['*'] = 1;
    priority['/'] = 1;
    priority['^'] = 2;

    for(int i = 0; i < infix.length() ;++i)//逐个读取中缀表达式字符串中的字符
    {
        current =infix[i].toLatin1();
        if(isdigit(current))//如果是数字直接输出
        {
            postfix.push_back(current);
            continue;
        }
        switch(current)
        {
        case '+':
        case '-':
        case '*':
        case '/':
        case '^':
            if(i > 0)    //如果运算符的前一项不是右括号则说明前一个数字输入完毕，用#标识前面几个字符组成一个数字
             {
                if (infix[i-1] != ')')
                    postfix.push_back('#');
                /*if(infix[i-1].isDigit())
                    postfix.push_back('#');
                else
                    throw "expression is illegality";*/
            }
            else
                postfix.push_back('#');
            if(!stack.empty())//比较目前符号与栈顶符号优先级，低于则出栈，并输出字符串
            {
                char tempTop=stack.top();
                while(tempTop != '(' && priority[current]<=priority[tempTop])
                {
                    stack.pop();
                    postfix.push_back(tempTop);
                    if(stack.empty())
                        break;
                    tempTop=stack.top();
                }
            }
            stack.push(current);//符号全部出栈或者遇到了'('或者大于栈顶符号的优先级，将新符号压入栈中
            break;
        case '.':
            postfix.push_back(current);
            break;
        case '%':
            postfix.push_back(current);
            break;
        case '(':
            stack.push(current);//左括号直接入栈
            break;
        case ')':
            postfix.push_back('#');//右括号说明前方数字输入完成，标识一下
            char tempTop;
            tempTop=stack.top();
            while(tempTop !='(')//直到栈顶元素是左括号才停止循环
            {
                stack.pop();
                postfix.push_back(tempTop);
                tempTop=stack.top();
            }
            stack.pop();
            break;
        default:
            throw "表达式中含有非法字符";
            break;
        }
    }
    if(infix[infix.size()-1] != ')')
    {
        if(infix[infix.size()-1].isDigit())
            postfix.push_back('#');
        else
            throw "表达式非法";
    }
    while(!stack.empty())
    {
        char tempOut=stack.top();
        stack.pop();
        postfix.push_back(tempOut);
    }
    return postfix;
}

double MainWindow::compute(QString s) throw(const char*)
{
    std::stack<double> stack;
    QString str("0");
    double curr;

    double  temNum1;
    double  temNum2;
    for(auto i = s.begin();i != s.end(); i++)
    {
        if((*i).isDigit())
        {
            str.push_back(*i);
            continue;
        }
        switch((*i).toLatin1())
        {
        case '+':
            temNum1=stack.top();
            stack.pop();
            temNum2=stack.top();
            stack.pop();
            stack.push(temNum2 + temNum1);
            break;
        case '-':
            temNum1=stack.top();
            stack.pop();
            temNum2=stack.top();
            stack.pop();
            stack.push(temNum2 - temNum1);
            break;
        case '*':
            temNum1=stack.top();
            stack.pop();
            temNum2=stack.top();
            stack.pop();
            stack.push(temNum2 * temNum1);
            break;
        case '/':
            temNum1=stack.top();
            stack.pop();
            temNum2=stack.top();
            stack.pop();
            stack.push(temNum2 / temNum1);
            break;
        case '^':
            temNum1=stack.top();
            stack.pop();
            temNum2=stack.top();
            stack.pop();
            stack.push(pow(temNum2, temNum1));
            break;
        case '.':
            str.push_back(*i);
            break;
        case '#':
            curr =str.toDouble();//字符串转换为浮点型
            str.clear();
            stack.push(curr);
            break;
        case '%':
            curr = stack.top();
            stack.pop();
            curr*=0.01;
            stack.push(curr);
            break;
        default:
            throw "表达式中含有非法字符";
            break;
        }
    }
    curr=stack.top();
    return curr;
}

QString MainWindow::currentText()
{
    QTextDocument *document = ui->textEdit->document();
    QTextBlock block = document->lastBlock();
    return block.text();
}

void MainWindow::setCurrentText(QString text)
{
    QString allText = ui->textEdit->toPlainText();
    int pos = allText.lastIndexOf('\n');
    if(pos >= 0)
        allText.replace(allText.lastIndexOf('\n') + 1, allText.size(), text);
    else
        allText = text;
    ui->textEdit->setText(allText);
    ui->textEdit->moveCursor(QTextCursor::End);
}

void MainWindow::on_action_2_triggered()
{
    ui->textEdit->setText(log);
    setCurrentText("0");
    waitForOperator = false;
}

void MainWindow::on_action_3_triggered()
{
    log.clear();
}

void MainWindow::on_action_triggered()
{
    QClipboard *board = QApplication::clipboard();
    QString text = board->text();
    ui->textEdit->append(text);
}

void MainWindow::initUi()
{
    ui->setupUi(this);
    setCurrentText("0");
}

void MainWindow::connectSlots()
{
    QPushButton *digitBtns[10] = {
            ui->digitBtn0,      ui->digitBtn1,          ui->digitBtn2,          ui->digitBtn3,
            ui->digitBtn4,      ui->digitBtn5,          ui->digitBtn6,          ui->digitBtn7,
            ui->digitBtn8,      ui->digitBtn9
        };
        for (auto btn : digitBtns)
            connect(btn, &QPushButton::clicked, this, &MainWindow::digitClicked);
        QPushButton *operatorBtns[7] = {
            ui->addBtn,         ui->subtractionBtn,     ui->multiplicationBtn,  ui->divisionBtn,
            ui->leftBracketBtn, ui->rightBracketBtn,    ui->powBtn
        };
        for (auto btn : operatorBtns)
            connect(btn, &QPushButton::clicked, this, &MainWindow::operatorClicked);
}

void MainWindow::setShortcutKeys()
{
    Qt::Key key[21] = {
        Qt::Key_0,          Qt::Key_1,          Qt::Key_2,          Qt::Key_3,
        Qt::Key_4,          Qt::Key_5,          Qt::Key_6,          Qt::Key_7,
        Qt::Key_8,          Qt::Key_9,
        Qt::Key_Plus,       Qt::Key_Minus,      Qt::Key_Asterisk,   Qt::Key_Slash,
        Qt::Key_Enter,      Qt::Key_Period,     Qt::Key_Backspace,  Qt::Key_M,
        Qt::Key_ParenLeft,  Qt::Key_ParenRight, Qt::Key_AsciiCircum
    };
    QPushButton *btn[21] = {
        ui->digitBtn0,      ui->digitBtn1,          ui->digitBtn2,          ui->digitBtn3,
        ui->digitBtn4,      ui->digitBtn5,          ui->digitBtn6,          ui->digitBtn7,
        ui->digitBtn8,      ui->digitBtn9,
        ui->addBtn,         ui->subtractionBtn,     ui->multiplicationBtn,  ui->divisionBtn,
        ui->equalBtn,       ui->pointBtn,           ui->clearBtn,           ui->signBtn,
        ui->leftBracketBtn, ui->rightBracketBtn,    ui->powBtn
    };
    for (int i = 0; i < 21; i++)
        btn[i]->setShortcut(QKeySequence(key[i]));
    ui->clearAllBtn->setShortcut(QKeySequence("Ctrl+Backspace"));

}
