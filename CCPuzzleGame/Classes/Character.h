#ifndef Character_h
#define Character_h

#include "cocos2d.h"

class Character : public cocos2d::Ref
{
public:
    // キャラクター属性
    enum class Element{
        Fire,
        Water,
        Wind,
        Holy,
        Shadow,
        None,
    };
    
protected:
    int _remaingTurn; // 攻撃するまでのターン
    CC_SYNTHESIZE(int, _hp, Hp); // ヒットポイント
    CC_SYNTHESIZE(int, _maxHP, MaxHp); // 最大ヒットポイント
    CC_SYNTHESIZE(int, _attack, Attack); // 攻撃力
    CC_SYNTHESIZE(Element, _element, Element); // 属性
    CC_PROPERTY(int, _turnCount, TurnCount); //攻撃ターン数(敵の場合)
    
public:
    Character();
    static Character* create(); // インスタンス生成
    
    float getHpPercentage();
    bool isAttackTurn();
    static int getDamage(int ballCount, int chainCount, Character* attacker, Character* defender);
    
protected:
    static float getElementBonus(Element attackElement, Element defenderElement); // 属性による攻撃ボーナス
};

#endif /* Character_h */
