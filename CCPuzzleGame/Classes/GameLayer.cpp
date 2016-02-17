#include "GameLayer.h"

#define BALL_NUM_X 6
#define BALL_NUM_Y 5
#define WINSIZE Director::getInstance()->getWinSize()
#define TAG_LEVEL_LAYER 10000

USING_NS_CC;

// コンストラクタ
GameLayer::GameLayer()
:_movingBall(nullptr)
,_movedBall(false)
,_touchable(true)
,_maxRemoveNo(0)
,_chainNumber(0)
,_level(0)
,_nextLevel(0)
{
    // 乱数初期化及び各ボールの出現の重みを指定
    std::random_device device;
    _engine = std::default_random_engine(device());
    _distForBall = std::discrete_distribution<int>{20, 20, 20, 20, 20, 10};
    _distForMember = std::uniform_int_distribution<int>(0, 4); // 0,1,2,3,4のいずれかの値を取得することが出来る
}

// シーンの生成
Scene* GameLayer::createScene(int level)
{
    auto scene = Scene::create();
    auto layer = GameLayer::create(level);
    scene->addChild(layer);
    
    return scene;
}

// インスタンス生成
GameLayer* GameLayer::create(int level)
{
    GameLayer *pRet = new GameLayer();
    pRet->init(level);
    pRet->autorelease();
    
    return pRet;
}

// 初期化
bool GameLayer::init(int level)
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
    
    _level = level;
    
    initBackGround(); // 背景の初期化
    initBalls(); // ボールの初期表示
    initEnemy(); // 敵の表示
    initMembers(); // メンバーの表示
    initLevelLayer(); // レベル表示レイヤーの表示
    
    return true;
}

// 背景の初期表示
void GameLayer::initBackGround()
{
    // キャラクター部の背景
    auto bgForCharacter = Sprite::create("Background1.png");
    bgForCharacter->setAnchorPoint(Point(0, 1));
    bgForCharacter->setPosition(Point(0, WINSIZE.height));
    addChild(bgForCharacter, ZOrder::BgForCharacter);
    
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

void GameLayer::initEnemy()
{
    // 敵の情報
    _enemyData = Character::create();
    _enemyData->retain();
    _enemyData->setMaxHp(100 * _level);
    _enemyData->setHp(100 * _level);
    _enemyData->setElement(Character::Element::Wind);
    _enemyData->setTurnCount(3);
    
    // 敵の表示
    _enemy = Sprite::create(StringUtils::format("Enemy%d.png", _level));
    _enemy->setPosition(Point(320, 660 + (WINSIZE.height - 660) / 2));
    addChild(_enemy, ZOrder::Enemy);
    
    // 敵のヒットボイントバーの枠
    auto hpBg = Sprite::create("HpEnemyBackground.png");
    hpBg->setPosition(Point(320, 530 + (WINSIZE.height - 660) / 2));
    addChild(hpBg, ZOrder::EnemyHp);
    
    // 敵ヒットポイントバーの表示
    _hpBarforEnemy = ProgressTimer::create(Sprite::create("HpEnemyRed.png"));
    _hpBarforEnemy->setPosition(Point(hpBg->getContentSize().width / 2, hpBg->getContentSize().height / 2));
    _hpBarforEnemy->setType(ProgressTimer::Type::BAR);
    _hpBarforEnemy->setBarChangeRate(Point(1, 0));
    _hpBarforEnemy->setPercentage(_enemyData->getHpPercentage());
    hpBg->addChild(_hpBarforEnemy);
}

void GameLayer::initMembers()
{
    std::vector<std::string> fileNames{
        "CardBlue.png",
        "CardRed.png",
        "CardGreen.png",
        "CardYellow.png",
        "CardPurple.png",
    };
    
    std::vector<Character::Element> elements{
        Character::Element::Water,
        Character::Element::Fire,
        Character::Element::Wind,
        Character::Element::Holy,
        Character::Element::Shadow,
    };
    
    for(int i = 0; i < fileNames.size(); ++i){
        auto memberData = Character::create();
        memberData->setMaxHp(200);
        memberData->setHp(200);
        memberData->setElement(elements[i]);
        _memberData.pushBack(memberData);
        
        Sprite* member = Sprite::create(fileNames[i].c_str());
        member->setPosition(Point(70 + i * 125, 598));
        addChild(member, ZOrder::Char);
        
        auto hpBg = Sprite::create("HpCardBackground.png");
        hpBg->setPosition(Point(70 + i * 125, 554));
        addChild(hpBg, ZOrder::CharHp);
        
        auto hpBarForMember = ProgressTimer::create(Sprite::create("HpCardGreen.png"));
        hpBarForMember->setPosition(Point(hpBg->getContentSize().width / 2, hpBg->getContentSize().height / 2));
        hpBarForMember->setType(ProgressTimer::Type::BAR);
        hpBarForMember->setMidpoint(Point::ZERO);
        hpBarForMember->setBarChangeRate(Point(1, 0));
        hpBarForMember->setPercentage(memberData->getHpPercentage());
        hpBg->addChild(hpBarForMember);
        
        // 配列に格納
        _members.pushBack(member);
        _hpBarForMembers.pushBack(hpBarForMember);
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
        int chainNum = 0;
        int damage = 0;
        int healing = 0;
        std::set<int> attackers;
        
        // ダメージ・回復量の計算
        calculateDamage(chainNum, healing, damage, attackers);
        
        // 敵にダメージを与える
        int afterHp = _enemyData->getHp() - damage;
        
        if(damage > 0){
            // アタック処理
            attackToEnemy(damage, attackers);
        }
        
        if(healing){
            // 回復量
            healMember(healing);
        }
        
        // 敵にダメージを与えた後の処理を設定
        if(afterHp > 0){
            // 敵のHPが残っている場合は敵のターンに移る
            CallFunc* func = CallFunc::create(CC_CALLBACK_0(GameLayer::attackFromEnemy, this));
            runAction(Sequence::create(DelayTime::create(0.5), func, nullptr));
        }
        
        CallFunc* func;
        if(afterHp <= 0){
            func = CallFunc::create(CC_CALLBACK_0(GameLayer::winAnimation, this));
        }else{
            func = CallFunc::create(CC_CALLBACK_0(GameLayer::attackFromEnemy, this));
        }
        runAction(Sequence::create(DelayTime::create(0.5), func, nullptr));
        
        // タップを有効にする Todo:
        //_touchable = true;
    }
}

void GameLayer::calculateDamage(int &chainNum, int &healing, int &damage, std::set<int> &attackers)
{
    auto removeIt = _removeNumbers.begin();
    while (removeIt != _removeNumbers.end()) {
        auto ballIt = (*removeIt).begin();
        while(ballIt != (*removeIt).end()){
            if((*ballIt).first == BallSprite::BallType::Pink){
                // 回復
                healing += 5;
            }else{
                // アタッカーのデータを繰り返す
                for(int i = 0; i < _memberData.size(); ++i){
                    // メンバー情報取得
                    auto memberData = _memberData.at(i);
                    
                    // メンバーのHPが0の場合は、以下の処理は行わない
                    if(memberData->getHp() <= 0){
                        continue;
                    }
                    
                    // 消されたボールとアタッカーの属性よりアタッカーの判定
                    if(isAttacker((*ballIt).first, memberData->getElement())){
                        // アタッカー情報の保持
                        attackers.insert(i);
                        
                        // ダメージ
                        damage += Character::getDamage((*ballIt).second, chainNum, memberData, _enemyData);
                        
                    }
                }
            }
            
            chainNum++;
            ballIt++;
        }
        removeIt++;
    }
}

// アタッカー判定
bool GameLayer::isAttacker(BallSprite::BallType type, Character::Element element)
{
    switch (type) {
        case BallSprite::BallType::Red:
            // 赤ボール：火属性
            if(element == Character::Element::Fire){
                return true;
            }
            break;
        case BallSprite::BallType::Blue:
            // 青ボール：水属性
            if(element == Character::Element::Water){
                return true;
            }
            break;
        case BallSprite::BallType::Green:
            // 緑ボール：風属性
            if(element == Character::Element::Wind){
                return true;
            }
            break;
        case BallSprite::BallType::Yellow:
            // 黄ボール：光属性
            if(element == Character::Element::Holy){
                return true;
            }
            break;
        case BallSprite::BallType::Purple:
            // 紫ボール：闇属性
            if (element == Character::Element::Shadow) {
                return true;
            }
            break;
        default:
            break;
    }
    return false;
}

// 敵への攻撃
void GameLayer::attackToEnemy(int damage, std::set<int> attackers)
{
    // 敵のHPを取得する
    float preHpPercentage = _enemyData->getHpPercentage();
    
    // ダメージをセットする
    int afterHp = _enemyData->getHp() - damage;
    if(afterHp < 0){ afterHp = 0; }
    _enemyData->setHp(afterHp);
    
    // 敵ヒットポイントバーのアニメーション
    auto act = ProgressFromTo::create(0.5, preHpPercentage, _enemyData->getHpPercentage());
    _hpBarforEnemy->runAction(act);
    
    // 敵の被ダメージアニメーション
    _enemy->runAction(vibratingAnimation(afterHp));
    
    // メンバーの攻撃アニメーション
    for(auto attacker : attackers){
        auto member = _members.at(attacker);
        member->runAction(Sequence::create(MoveBy::create(0.1, Point(0,10)),MoveBy::create(0.1,Point(0,-10)),nullptr));
    }
}

// メンバーの回復
void GameLayer::healMember(int healing)
{
    for(int i = 0; i < _memberData.size(); ++i){
        // メンバーデータ取得
        auto memberData = _memberData.at(i);
        
        // HPが0の場合は回復しない
        if(memberData->getHp() <= 0){
            continue;
        }
        
        // メンバーを回復する
        float preHpPercentage = memberData->getHpPercentage();
        int afterHp = memberData->getHp() + healing;
        if(afterHp > memberData->getMaxHp()){
            afterHp = memberData->getMaxHp();
        }
        memberData->setHp(afterHp);
        
        // メンバーHPアニメーション
        auto act = ProgressFromTo::create(0.5, preHpPercentage, memberData->getHpPercentage());
        _hpBarForMembers.at(i)->runAction(act);
    }
}

// 敵からの攻撃
void GameLayer::attackFromEnemy()
{
    if(!_enemyData->isAttackTurn()){
        // 敵の攻撃ターンでない場合は、一連の攻撃処理を終わらせる
        endAnimation();
        return;
    }
    
    // メンバーを一人選択
    int index;
    Character* memberData;
    
    do{
        // ランダムでメンバーを選択
        index = _distForMember(_engine);
        memberData = _memberData.at(index);
        
        // HPが0のメンバーを選択した場合は再度選択しなおす
    }while (memberData->getHp() <= 0);
    
    auto member = _members.at(index);
    auto hpBarForMember = _hpBarForMembers.at(index);
    
    // メンバーにダメージを与える
    float preHpPercentage = memberData->getHpPercentage();
    int afterHp = memberData->getHp() - 25;
    if(afterHp > memberData->getMaxHp()) { afterHp = memberData->getMaxHp();}
    memberData->setHp(afterHp);
    
    // メンバーヒットポイントバーのアニメーション
    auto act = ProgressFromTo::create(0.5, preHpPercentage, memberData->getHpPercentage());
    hpBarForMember->runAction(act);
    
    // メンバーの被ダメージアニメーション
    member->runAction(vibratingAnimation(afterHp));
    
    // 敵の攻撃アニメーション
    auto seq = Sequence::create(MoveBy::create(0.1, Point(0,-10)), MoveBy::create(0.1, Point(0,10)), nullptr);
    _enemy->runAction(seq);
    
    // 味方の全滅チェック
    bool allHpZero = true;
    for(auto character : _memberData){
        if(character->getHp() > 0){
            allHpZero = false;
            break;
        }
    }
    
    // アニメーション終了処理
    CallFunc* func;
    if(allHpZero){
        func = CallFunc::create(CC_CALLBACK_0(GameLayer::loseAnimation, this));
    }else{
        func = CallFunc::create(CC_CALLBACK_0(GameLayer::endAnimation, this));
    }
    runAction(Sequence::create(DelayTime::create(0.5), func, nullptr));
}

// レベル表示レイヤー
void GameLayer::initLevelLayer()
{
    // レベルレイヤーの生成
    auto levelLayer = LayerColor::create(Color4B(0, 0, 0, 191), WINSIZE.width, WINSIZE.height);
    levelLayer->setPosition(Point::ZERO);
    levelLayer->setTag(TAG_LEVEL_LAYER);
    addChild(levelLayer, ZOrder::Level);
    
    // レベルの表示
    auto levelSprite = Sprite::create("Level.png");
    levelSprite->setPosition(Point(WINSIZE.width * 0.45, WINSIZE.height * 0.5));
    levelLayer->addChild(levelSprite);
    
    // レベル数の表示
    auto levelNumPath = StringUtils::format("%d.png", _level);
    auto levelNumSprite = Sprite::create(levelNumPath.c_str());
    levelNumSprite->setPosition(Point(WINSIZE.width * 0.85, WINSIZE.height * 0.5));
    levelLayer->addChild(levelNumSprite);
    
    // 1.5秒後に消えるようにする
    scheduleOnce(schedule_selector(GameLayer::removeLevelLayer), 1.5);
}

// レベル表示レイヤー
void GameLayer::removeLevelLayer(float dt)
{
    // タップ可能とする
    _touchable = true;
    
    // 0.5秒で消えるようにする
    auto levelLayer = getChildByTag(TAG_LEVEL_LAYER);
    levelLayer->runAction(Sequence::create(FadeTo::create(0.5, 0), RemoveSelf::create(), nullptr));
}

// Winアニメーション
void GameLayer::winAnimation()
{
    // 白い背景を用意する
    auto whiteLayer = LayerColor::create(Color4B(255,255,255,127), WINSIZE.width, WINSIZE.height);
    whiteLayer->setPosition(Point::ZERO);
    addChild(whiteLayer, ZOrder::Result);
    
    // Win画像を表示する
    auto win = Sprite::create("Win.png");
    win->setPosition(Point(WINSIZE.width / 2, WINSIZE.height / 2));
    addChild(win, ZOrder::Result);
    
    // 次のレベルを設定(Level3の次はないので1に戻る)
    if(_level >= 3){
        _nextLevel = 1;
    }else{
        _nextLevel = _level + 1;
    }
    
    // 指定秒数後に次のシーンへ
    scheduleOnce(schedule_selector(GameLayer::nextScene), 3);
}

// Loseアニメーション
void GameLayer::loseAnimation()
{
    // 黒い背景を用意する
    auto blackLayer = LayerColor::create(Color4B(0,0,0,127),WINSIZE.width, WINSIZE.height);
    blackLayer->setPosition(Point::ZERO);
    addChild(blackLayer, ZOrder::Result);
    
    // Lose画像を表示する
    auto lose = Sprite::create("Lose.png");
    lose->setPosition(Point(WINSIZE.width / 2, WINSIZE.height / 2));
    addChild(lose, ZOrder::Result);
    
    // 次のレベルを指定
    _nextLevel = 1;
    
    // 指定秒数後に次のシーンへ
    scheduleOnce(schedule_selector(GameLayer::nextScene), 3);
}

// 次のシーンへ遷移
void GameLayer::nextScene(float dt)
{
    // 次のシーンを生成する
    auto scene = GameLayer::createScene(_nextLevel);
    Director::getInstance()->replaceScene(scene);
}

// アニメーション終了時処理
void GameLayer::endAnimation()
{
    // タップを有効にする
    _touchable = true;
}

// 振動アニメーション
Spawn* GameLayer::vibratingAnimation(int afterHp)
{
    // 振動アニメーション
    auto move = Sequence::create(MoveBy::create(0.025, Point(5, 5)),
                                 MoveBy::create(0.025, Point(-5, -5)),
                                 MoveBy::create(0.025, Point(-5, -5)),
                                 MoveBy::create(0.025, Point(5, 5)),
                                 MoveBy::create(0.025, Point(5, -5)),
                                 MoveBy::create(0.025, Point(-5, 5)),
                                 MoveBy::create(0.025, Point(-5, 5)),
                                 MoveBy::create(0.025, Point(5, -5)),
                                 nullptr);
    
    // ダメージ時に色を赤くする
    Action* tint;
    if(afterHp > 0){
        // HPが0より大きい場合は元の色に戻す
        tint = Sequence::create(TintTo::create(0, 255, 0, 0),
                                DelayTime::create(0.2),
                                TintTo::create(0, 255, 255, 255),
                                NULL);
    }else{
        // HPが0の場合は赤いままにする
        tint = TintTo::create(0, 255, 0, 0);
    }
    
    return Spawn::create(move, tint, nullptr);
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




