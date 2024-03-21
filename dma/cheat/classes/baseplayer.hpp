#pragma once

#include <Windows.h>
#include "../math/vector3.hpp"
#include "../../DMALibrary/Memory/Memory.h"

#include <list>

enum BONEINDEX : DWORD
{
	head = 6,
	neck_0 = 5,
	spine_1 = 4,
	spine_2 = 2,
	pelvis = 0,
	arm_upper_L = 8,
	arm_lower_L = 9,
	hand_L = 10,
	arm_upper_R = 13,
	arm_lower_R = 14,
	hand_R = 15,
	leg_upper_L = 22,
	leg_lower_L = 23,
	ankle_L = 24,
	leg_upper_R = 25,
	leg_lower_R = 26,
	ankle_R = 27,
};

struct boneJointData
{
	vector3 position;
	char pad[ 0x14 ];
};

struct boneJointPos
{
	vector3 position;
};

namespace boneJointList
{
	// ¼¹¹Ç
	inline std::list<DWORD> Trunk = { head,neck_0,spine_2, pelvis };
	// ×ó±Û
	inline std::list<DWORD> LeftArm = { neck_0,  arm_upper_L, arm_lower_L, hand_L };
	// ÓÒ±Û
	inline std::list<DWORD> RightArm = { neck_0, arm_upper_R,arm_lower_R, hand_R };
	// ×óÍÈ	
	inline std::list<DWORD> LeftLeg = { pelvis, leg_upper_L , leg_lower_L, ankle_L };
	// ÓÒÍÈ
	inline std::list<DWORD> RightLeg = { pelvis, leg_upper_R , leg_lower_R, ankle_R };
	// ×ÜÁÐ±í
	inline std::vector<std::list<DWORD>> List = { Trunk, LeftArm, RightArm, LeftLeg, RightLeg };
}

struct BoneConnection
{
	int bone1;
	int bone2;

	BoneConnection( int b1, int b2 ) : bone1( b1 ), bone2( b2 ) {}
};

inline BoneConnection boneConnections[ ] = {
	BoneConnection( 5, 4 ),  // neck to spine
	BoneConnection( 4, 0 ),  // spine to hip
	BoneConnection( 4, 8 ), // spine to left shoulder
	BoneConnection( 8, 9 ), // left shoulder to left arm
	BoneConnection( 9, 11 ), // arm to hand
	BoneConnection( 4, 13 ), // spine to shoulder
	BoneConnection( 13, 14 ), // shoulder to right arm
	BoneConnection( 14, 16 ), // right arm to right hand
	BoneConnection( 4, 0 ),  // spine to spine_1
	BoneConnection( 0, 23 ), // left_hip to left_knee
	BoneConnection( 23, 24 ), // left knee to left foot
	BoneConnection( 0, 26 ), // right_hip to right_knee
	BoneConnection( 26, 27 ) // right knee to right foot
};

namespace sdk
{
	class basePlayer
	{
		int team;

		int health;

		int flags;

		vector3 origin;

		vector3 head;

		vector3 viewAngles;

		std::string stringName;
		char name[ 256 ];

		std::uintptr_t address;

		std::uintptr_t nameData;

		std::uintptr_t gameSceneNode;
		std::uintptr_t boneArray;

		std::vector<boneJointPos> bonePosList;

		boneJointData bonesArray[ 30 ];

	public:
		basePlayer( std::uintptr_t address, VMMDLL_SCATTER_HANDLE handle );

		basePlayer( std::uintptr_t address, std::uintptr_t pawn, VMMDLL_SCATTER_HANDLE handle );

		~basePlayer( );

		void preCache( VMMDLL_SCATTER_HANDLE handle );

		void cachePawn( VMMDLL_SCATTER_HANDLE handle );


		vector3 getPosition( );

		void updatePosition( VMMDLL_SCATTER_HANDLE handle );

		std::string getName( );

		bool isValid( );

		bool isAlive( );
		int getHealth( );
		void updateHealth( VMMDLL_SCATTER_HANDLE handle );

		void updateFlags( VMMDLL_SCATTER_HANDLE handle );
		int getFlags( );

		void updateViewAngles( VMMDLL_SCATTER_HANDLE handle );
		vector3 getViewAngles( );

		int getTeam( );

		void updatePawn( VMMDLL_SCATTER_HANDLE handle );
		std::uintptr_t getPawn( );
		std::uintptr_t getAddress( );
		
		void updateGameScene( VMMDLL_SCATTER_HANDLE handle, std::uintptr_t override );
		void updateBoneArray( VMMDLL_SCATTER_HANDLE handle, std::uintptr_t override );
		void getBoneArrayList( VMMDLL_SCATTER_HANDLE handle, std::uintptr_t override );
		
		void updateGameScene( VMMDLL_SCATTER_HANDLE handle );
		void updateBoneArray( VMMDLL_SCATTER_HANDLE handle );
		void getBoneArrayList( VMMDLL_SCATTER_HANDLE handle );
		
		bool updateBoneData( );

		std::vector<boneJointPos> boneList( ) { return bonePosList; }

	public:
		std::uintptr_t pawn;
		std::uintptr_t prePawn;

		std::uintptr_t networkedData;

	};
}