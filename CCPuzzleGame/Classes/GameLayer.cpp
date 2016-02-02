#include "GameLayer.h"

#define BALL_NUM_X 6
#define BALL_NUM_Y 5

USING_NS_CC;

// コンストラクタ
GameLayer::GameLayer()
:_movingBall(nullptr)
,_movedBall(false)
,_touchable(true)
,_maxRemoveNo(0)
,_chainNumber(0)
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
            newBall(BallSprite::PositionIndex(x, y), true);
        }
    }
}

// 新規ボールの作成
BallSprite* GameLayer::newBall(BallSprite::PositionIndex positionIndex, bool visible)
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
    auto ball = BallSprite::create((BallSprite::BallType)ballType, visible);
    ball->setPositionIndexAndChangePosition(positionIndex);
    addChild(ball, ZOrder::Ball);
    
    return ball;
}

bool GameLayer::onTouchBegan(Touch *touch, Event *unused_event)
{
    if(!_touchable){ return false; }
    
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
    
    // 一列に並んだボールがあるかチェックする
    _chainNumber = 0;
    _removeNumbers.clear();
    checksLinedBalls();
}

// 一列に並んだボールがあるかをチェックする
void GameLayer::checksLinedBalls()
{
    // 画面をタップ不可にする
    _touchable = false;
    
    if(existsLinedBalls()){
        // 3個以上並んだボールが存在する場合
        
        // 連鎖カウントアップ
        _chainNumber++;
        
        // ボールの削除と生成
        removeAndGenerateBalls();
        
        // アニメーション後に再チェック
        auto delay = DelayTime::create(ONE_ACTION_TIME * (_maxRemoveNo + 1));
        auto func = CallFunc::create(CC_CALLBACK_0(GameLayer::checksLinedBalls, this));
        auto seq = Sequence::create(delay, func, nullptr);
        runAction(seq);
    }else{
        // タップを有効にする
        _touchable = true;
    }
}

// 3個以上並んだボールの存在をチェック
bool GameLayer::existsLinedBalls()
{
    // ボールのパラメータを初期化する
    initBallParams();
    
    // 消去される順番の初期化
    _maxRemoveNo = 0;
    
    for(int x = 1; x <= BALL_NUM_X; ++x){
        for(int y = 1; y <= BALL_NUM_Y; ++y){
            // x方向のボールをチェック
            checkedBall(BallSprite::PositionIndex(x, y), Direction::_x);
            // y方向のボールをチェック
            checkedBall(BallSprite::PositionIndex(x, y), Direction::_y);
        }
    }
    
    // 戻り値の決定
    return _maxRemoveNo > 0;
}

// 全てのボールのBallTypeを取得
Map<int, BallSprite*> GameLayer::getAllBalls()
{
    auto balls = Map<int, BallSprite*>();
    
    for(auto object : getChildren()){
        auto ball = dynamic_cast<BallSprite*>(object);
        if(ball){
            balls.insert(ball->getTag(), ball);
        }
    }
    return balls;
}

// 指定方向のボールと同じ色かチェックする
bool GameLayer::isSameBallType(BallSprite::PositionIndex current, Direction direction)
{
    // 全てのボールのBallTypeを取得
    auto allBalls = getAllBalls();
    
    if(direction == Direction::_x){
        if(current._x + 1 > BALL_NUM_X){
            // 列が存在しない場合は抜ける
            return false;
        }
    }else{
        if(current._y + 1 > BALL_NUM_Y){
            // 行が存在しない場合は抜ける
            return false;
        }
    }
    
    // 現在のボールを取得
    int currentTag = BallSprite::generateTag(BallSprite::PositionIndex(current._x, current._y));
    BallSprite* currentBall = allBalls.at(currentTag);
    
    // 次のボールを取得
    int nextTag;
    if(direction == Direction::_x){
        nextTag = BallSprite::generateTag(BallSprite::PositionIndex(current._x + 1, current._y));
    }else{
        nextTag = BallSprite::generateTag(BallSprite::PositionIndex(current._x, current._y + 1));
    }
    BallSprite* nextBall = allBalls.at(nextTag);
    
    if(!nextBall) return false;
    if(currentBall->getBallType() == nextBall->getBallType()){
        // 次のボールが同じBallTypeである
        return true;
    }
    
    return false;
}

// ボールのパラメータを初期化する
void GameLayer::initBallParams()
{
    // 全てのボールのBallTypeを取得
    auto allBalls = getAllBalls();
    
    for(auto ball : allBalls){
        ball.second->resetParams();
    }
}

// 全ボールに対してボールの並びをチェックする
void GameLayer::checkedBall(BallSprite::PositionIndex current, Direction direction)
{
    // 全てのボールのBallTypeを取得する
    auto allBalls = getAllBalls();
    
    // 検索するタグの生成
    int tag = BallSprite::generateTag(BallSprite::PositionIndex(current._x, current._y));
    BallSprite* ball = allBalls.at(tag);
    if(!ball) return;
    
    // 指定方向のチェック済みフラグを取得
    bool checked;
    if(direction == Direction::_x){
        checked = ball->getCheckedX();
    }else{
        checked = ball->getCheckedY();
    }
    
    // 未チェックのボールのチェックをする
    if(!checked){
        int num = 0;
        while(true){
            // 検索位置を取得
            BallSprite::PositionIndex searchPosition;
            if(direction == Direction::_x){
                searchPosition = BallSprite::PositionIndex(current._x + num, current._y);
            }else{
                searchPosition = BallSprite::PositionIndex(current._x, current._y + num);
            }
            
            // 次のボールと同じBallTypeかチェックする
            if(isSameBallType(searchPosition, direction)){
                // 次のボールと同じBallType
                int nextTag = BallSprite::generateTag(searchPosition);
                auto nextBall = allBalls.at(nextTag);
                
                // チェックしたボールのチェック済みフラグを立てる
                if(direction == Direction::_x){
                    nextBall->setCheckedX(true);
                }else{
                    nextBall->setCheckedY(true);
                }
                num++;
            }else{
                // 次のボールが異なる場合
                // numが2以上ということは3個以上ボールが並んでいる状態
                if(num >= 2){
                    // アニメーションにおいて消える順番を示す
                    int removeNo = 0;
                    
                    if(_removeNumbers.size() <= _chainNumber){
                        // 配列が存在しない場合は追加する
                        std::map<BallSprite::BallType, int> removeNumber;
                        _removeNumbers.push_back(removeNumber);
                    }
                    // 消したボールの個数を記録する
                    _removeNumbers[_chainNumber][ball->getBallType()] += num + 1;
                    
                    // すでにRemoveNoがあるものが存在するかチェックする
                    for(int i = 0; i <= num; ++i){
                        BallSprite::PositionIndex linedPosition;
                        if(direction == Direction::_x){
                            linedPosition = BallSprite::PositionIndex(current._x + i, current._y);
                        }else{
                            linedPosition = BallSprite::PositionIndex(current._x, current._y + i);
                        }
                        
                        int linedBallTag = BallSprite::generateTag(linedPosition);
                        auto linedBall = allBalls.at(linedBallTag);
                        if(linedBall->getRemoveNo() > 0){
                            // すでにRemoveNoがあるものが存在するのでremoveNoを取得し次の処理を行う
                            removeNo = linedBall->getRemoveNo();
                            break;
                        }
                    }
                    
                    // 削除する順番のカウントアップ
                    if(removeNo == 0){
                        removeNo = ++_maxRemoveNo;
                    }
                    
                    // 3個以上並んでいた場合はremoveNoをセットする
                    for(int i = 0; i <= num; ++i){
                        BallSprite::PositionIndex linedPosition;
                        if(direction == Direction::_x){
                            linedPosition = BallSprite::PositionIndex(current._x + i, current._y);
                        }else{
                            linedPosition = BallSprite::PositionIndex(current._x, current._y + i);
                        }
                        int linedBallTag = BallSprite::generateTag(linedPosition);
                        auto linedBall = allBalls.at(linedBallTag);
                        linedBall->setRemoveNo(removeNo);
                    }
                }
                break;
            }
        }
        
        // 指定方向をチェック済みとする
        if(direction == Direction::_x){
            ball->setCheckedX(true);
        }else{
            ball->setCheckedY(true);
        }
    }
}

// ボールの削除とボールの生成
void GameLayer::removeAndGenerateBalls()
{
    // 全てのボールのBallTypeを取得
    auto allBalls = getAllBalls();
    
    int maxRemoveNo = 0;
    
    for(int x  = 1; x <= BALL_NUM_X; ++x){
        int fallCount = 0;
        for(int y = 1; y <= BALL_NUM_Y; ++y){
            int tag = BallSprite::generateTag(BallSprite::PositionIndex(x, y));
            auto ball = allBalls.at(tag);
            
            if(ball){
                int removeNoForBall = ball->getRemoveNo();
                
                if(removeNoForBall > 0){
                    // 落ちる段数をカウント
                    // ボールが1つ消えるため、それより上にあるボールはプラス1段落下することになる
                    fallCount++;
                    
                    if(removeNoForBall > maxRemoveNo){
                        maxRemoveNo = removeNoForBall;
                    }
                }else{
                    // 落ちる段数をリセット
                    ball->setFallCount(fallCount);
                }
            }
        }
        // ボールを生成する
        generateBalls(x, fallCount);
    }
    // ボールの消去&落下アニメーション
    animationBalls();
}

// ボールを生成する
void GameLayer::generateBalls(int xLineNum, int fallCount)
{
    for(int i = 1; i <= fallCount; ++i){
        // ボールを生成する
        auto positionIndex = BallSprite::PositionIndex(xLineNum, BALL_NUM_Y + i);
        auto ball = newBall(positionIndex, false);
        ball->setFallCount(fallCount);
    }
}

// ボールの削除と落下アニメーション
void GameLayer::animationBalls()
{
    // 全てのボールのBallTypeを取得
    auto allBalls = getAllBalls();
    
    for(auto ball : allBalls){
        // ボールのアニメーションを実行する
        ball.second->removingAndFallngAnimation(_maxRemoveNo);
    }
}




