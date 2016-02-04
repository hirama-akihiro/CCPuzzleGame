#ifndef GameLayer_h
#define GameLayer_h

#include "cocos2d.h"
#include <random>
#include "BallSprite.h"
#include "Character.h"

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
    bool _touchable; // タップの可否
    int _maxRemoveNo; // 一度に削除される最大連鎖の番号
    int _chainNumber; // 連続数のカウント
    std::vector<std::map<BallSprite::BallType, int>> _removeNumbers; // 削除するボールのカウント
    
    Character* _enemyData;
    cocos2d::Sprite* _enemy;
    cocos2d::ProgressTimer* _hpBarforEnemy;
    
    cocos2d::Vector<Character*> _memberData;
    cocos2d::Vector<cocos2d::Sprite*> _members;
    cocos2d::Vector<cocos2d::ProgressTimer*> _hpBarForMembers;
    
    void initBackGround(); // 背景の初期化
    void initBalls(); // ボールの初期表示
    BallSprite* newBall(BallSprite::PositionIndex positionIndex, bool visible); // 新規ボール作成
    BallSprite* getTouchBall(cocos2d::Point touchPos, BallSprite::PositionIndex withoutPosIndex = BallSprite::PositionIndex()); // タッチしたボールを取得
    void movedBall(); // タップ操作によるボールの移動完了時処理
    void checksLinedBalls(); // 一列に並んだボールがあるかをチェックする
    bool existsLinedBalls(); // 3個以上並んだボールの存在をチェック
    cocos2d::Map<int, BallSprite*> getAllBalls(); // 全てのボールのBallTypeを取得
    bool isSameBallType(BallSprite::PositionIndex current, Direction direction); // 指定方向のボールと同じ色かチェック
    void initBallParams(); // ボールのパラメータを初期化する
    void checkedBall(BallSprite::PositionIndex current, Direction direction); // 全ボールに対してボールの並びをチェック
    void removeAndGenerateBalls(); // ボールの削除とボールの生成
    void generateBalls(int xLineNum, int fallCount); // ボールを生成する
    void animationBalls(); // ボールの削除と落下アニメーション
    
    void initEnemy();
    void initMembers();
    
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
