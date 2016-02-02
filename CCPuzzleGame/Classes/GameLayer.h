#ifndef GameLayer_h
#define GameLayer_h

#include "cocos2d.h"
#include <random>
#include "BallSprite.h"

class GameLayer : public cocos2d::Layer
{
protected:
    // ボールのチェック方向
    enum class Direction{
        _x,
        _y,
    };
    
    // Zオーダー
    enum ZOrder{
        BgForCharacter = 0,
        BgForPuzzle = 1,
        Enemy = 2,
        EnemyHp = 3,
        Char = 4,
        CharHp = 5,
        Ball = 6,
        Level = 7,
        Result = 8,
    };
    
    std::default_random_engine _engine; // 乱数生成エンジン
    std::discrete_distribution<int> _distForBall; // 乱数の分布
    BallSprite* _movingBall; // 動かしているボール
    bool _movedBall; // 他のボールとの接触有無
    
    void initBackGround(); // 背景の初期化
    void initBalls(); // ボールの初期表示
    BallSprite* newBall(BallSprite::PositionIndex positionIndex); // 新規ボール作成
    BallSprite* getTouchBall(cocos2d::Point touchPos, BallSprite::PositionIndex withoutPosIndex = BallSprite::PositionIndex()); // タッチしたボールを取得
    void movedBall(); // タップ操作によるボールの移動完了時処理
    
public:
    GameLayer();
    virtual bool init(); // 初期化
    CREATE_FUNC(GameLayer); // crete関数作成
    // GameLayerクラスのインスタンスを持つシーンを生成する
    static cocos2d::Scene* createScene(); // シーン生成
    
    // タップイベント
    virtual bool onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* unused_event);
    virtual void onTouchMoved(cocos2d::Touch* touch, cocos2d::Event* unused_event);
    virtual void onTouchEnded(cocos2d::Touch* touch, cocos2d::Event* unused_event);
    virtual void onTouchCancelled(cocos2d::Touch* touch, cocos2d::Event* unused_event);
};

#endif /* GameLayer_h */
