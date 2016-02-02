#include "GameLayer.h"

#define BALL_NUM_X 6
#define BALL_NUM_Y 5

USING_NS_CC;

// コンストラクタ
GameLayer::GameLayer()
:_movingBall(nullptr)
,_movedBall(false)
{
    // 乱数初期化及び各ボールの出現の重みを指定
    std::random_device device;
    _engine = std::default_random_engine(device());
    _distForBall = std::discrete_distribution<int>{20, 20, 20, 20, 20, 10};
}

// シーンの生成
Scene* GameLayer::createScene()
{
    auto scene = Scene::create();
    auto layer = GameLayer::create();
    scene->addChild(layer);
    
    return scene;
}

// 初期化
bool GameLayer::init()
{
    if(!Layer::init()) return false;
    
    // シングルタップイベントの取得
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(_swallowsTouches); // 優先度の高いリスナーだけ実行
    
    // イベント関数の割り当て
    listener->onTouchBegan = CC_CALLBACK_2(GameLayer::onTouchBegan, this);
    listener->onTouchMoved = CC_CALLBACK_2(GameLayer::onTouchMoved, this);
    listener->onTouchEnded = CC_CALLBACK_2(GameLayer::onTouchEnded, this);
    listener->onTouchCancelled = CC_CALLBACK_2(GameLayer::onTouchCancelled, this);
    
    // イベントを追加する
    getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, this);
    
    initBackGround(); // 背景の初期化
    initBalls(); // ボールの初期表示
    
    return true;
}

// 背景の初期表示
void GameLayer::initBackGround()
{
    // パズル部の背景
    auto bgForPuzzle = Sprite::create("Background2.png");
    bgForPuzzle->setAnchorPoint(Point::ZERO);
    bgForPuzzle->setPosition(Point::ZERO);
    addChild(bgForPuzzle, ZOrder::BgForPuzzle);
}

// ボールの初期表示
void GameLayer::initBalls()
{
    for(int x = 0; x <= BALL_NUM_X; ++x){
        for(int y = 0; y <= BALL_NUM_Y; ++y){
            // ボールを生成する
            newBall(BallSprite::PositionIndex(x, y));
        }
    }
}

// 新規ボールの作成
BallSprite* GameLayer::newBall(BallSprite::PositionIndex positionIndex)
{
    // 現在のタグを取得
    int currentTag = BallSprite::generateTag(positionIndex);
    
    // 乱数を元に、ランダムでタイプを取得
    int ballType;
    while (true) {
        ballType = _distForBall(_engine);
        
        if(currentTag <= 1){ break; }
        
        // 妥当性のチェック(ボールが隣合わないようにする)
        
        // 左隣のボール
        auto ballX1Tag = currentTag - 10; // 1つ左は10引いた値
        auto ballX2Tag = currentTag - 20; // 2つ左は20引いた値
        auto ballX1 = (BallSprite*) getChildByTag(ballX1Tag);
        auto ballX2 = (BallSprite*) getChildByTag(ballX2Tag);
        
        // 現在のボールが1つ左と2つ左のボールと同じだとNG
        if(!(ballX1 && ballType == (int)ballX1->getBallType()) ||
           !(ballX2 && ballType == (int)ballX2->getBallType())){
            // 下隣のボール
            auto ballY1Tag = currentTag -1; // 1つ下は1引いた値
            auto ballY2Tag = currentTag -2; // 2つ下は2引いた値
            auto ballY1 = (BallSprite*) getChildByTag(ballY1Tag);
            auto ballY2 = (BallSprite*) getChildByTag(ballY2Tag);
            
            // 現在のボールが1つ下と2つ下のボールと同じだとNG
            if(!(ballY1 && ballType == (int)ballY1->getBallType()) ||
               !(ballY2 && ballType == (int)ballY2->getBallType())){
                // 左隣と下隣が揃わない場合はループを抜ける
                break;
            }
        }
    }
    
    // ボールの表示
    auto ball = BallSprite::create((BallSprite::BallType)ballType, true);
    ball->setPositionIndexAndChangePosition(positionIndex);
    addChild(ball, ZOrder::Ball);
    
    return ball;
}

bool GameLayer::onTouchBegan(Touch *touch, Event *unused_event)
{
    _movedBall = false;
    _movingBall = getTouchBall(touch->getLocation());
    
    if(_movingBall) { return true; }
    else{ return false; }
}

void GameLayer::onTouchMoved(Touch *touch, Event *unused_event)
{
    // スワイプとともにボールを移動する
    _movingBall->setPosition(_movingBall->getPosition() + touch->getDelta());
    
    auto touchBall = getTouchBall(touch->getLocation(), _movingBall->getPositionIndex());
    if(touchBall && _movingBall != touchBall){
        // 移動しているボールが別のボールの位置に移動
        _movedBall = true;
        
        // 別のボールの位置インデックスを取得(新たに触れたボールの位置インデックスを取得する)
        auto touchBallPositionIndex = touchBall->getPositionIndex();
        
        // 別のボールを移動しているボールの元の位置へ移動する
        touchBall->setPositionIndexAndChangePosition(_movingBall->getPositionIndex());
        
        // 移動しているボールの情報を変更
        _movingBall->setPositionIndex(touchBallPositionIndex);
    }
}

void GameLayer::onTouchEnded(Touch *touch, Event *unused_event)
{
    // タップ操作によるボールの移動完了時の処理
    movedBall();
}

void GameLayer::onTouchCancelled(Touch *touch, Event *unused_event)
{
    onTouchEnded(touch, unused_event);
}

// タップした位置のチェック
BallSprite* GameLayer::getTouchBall(Point touchPos, BallSprite::PositionIndex withoutPosIndex)
{
    for(int x = 1; x <= BALL_NUM_X; ++x){
        for(int y = 1; y <= BALL_NUM_Y; ++y){
            if(x == withoutPosIndex._x && y == withoutPosIndex._y){
                // 指定位置のボールの場合は以下の処理を行わない
                continue;
            }
            
            // タップ位置にあるボールかどうかを判断する
            int tag = BallSprite::generateTag(BallSprite::PositionIndex(x, y));
            auto ball = (BallSprite*)getChildByTag(tag);
            if(ball){
                // 2点間の距離を求める
                float distance = ball->getPosition().getDistance(touchPos);
                
                // ボールの当たり判定は円形。
                // つまりボールの中心からの半径で判断する
                if(distance < BALL_SIZE / 2){
                    // タップした位置にボールが存在する
                    return ball;
                }
            }
        }
    }
    return nullptr;
}

// タップ操作によるボールの移動完了時処理
void GameLayer::movedBall()
{
    // 移動しているボールを本来の位置に戻す
    _movingBall->resetPosition();
    _movingBall = nullptr;
}