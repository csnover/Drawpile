// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef CANVAS_ACL_STATE_H
#define CANVAS_ACL_STATE_H

#include <QObject>
#include <QVector>

namespace rustpile {
	struct PaintEngine;
	enum class Tier : uint8_t;
	struct UserACLs;
	struct FeatureTiers;
}

namespace canvas {

// Features with configurable access levels
enum class Feature {
	PutImage,
	RegionMove,
	Resize,
	Background,
	EditLayers,
	OwnLayers,
	CreateAnnotation,
	Laser,
	Undo,
	Metadata,
	Timeline
};

static const int FeatureCount = int(Feature::Timeline)+1;

/**
 * Access control list state that is relevant to the UI.
 */
class AclState : public QObject {
	Q_OBJECT
public:
	struct Layer {
		bool locked;              // layer is locked for all users
		rustpile::Tier tier;      // layer access tier (if not exclusive)
		QVector<uint8_t> exclusive; // if not empty, only these users can draw on this layer

		Layer();
		bool operator!=(const Layer &other) const;
	};

	explicit AclState(QObject *parent=nullptr);
	~AclState();

	void setLocalUserId(uint8_t localUser);

	void updateUserBits(const rustpile::UserACLs &acls);
	void updateFeatures(const rustpile::FeatureTiers &features);
	void updateLayers(rustpile::PaintEngine *pe);

	//! Is this user an operator?
	bool isOperator(uint8_t userId) const;

	//! Is this user trusted?
	bool isTrusted(uint8_t userId) const;

	//! Is this user locked (via user specific lock)
	bool isLocked(uint8_t userId) const;

	//! Is the local user an operator?
	bool amOperator() const;

	//! Is the local user locked (via user specific lock or general lock)
	bool amLocked() const;

	//! Is the local user trusted?
	bool amTrusted() const;

	//! Is the given layer locked for this user (ignoring canvaswide lock)
	bool isLayerLocked(uint16_t layerId) const;

	//! Is this feature available for us?
	bool canUseFeature(Feature f) const;

	//! Get the ID of the local user
	uint8_t localUserId() const;

	//! Get the required access tier to use this feature
	rustpile::FeatureTiers featureTiers() const;

	//! Get the layer's access control list
	Layer layerAcl(uint16_t layerId) const;

signals:
	//! Local user's operator status has changed
	void localOpChanged(bool op);

	//! Local user's lock status (either via user specific lock or general lock) has changed
	void localLockChanged(bool locked);

	//! The ACL for this layer has changed
	void layerAclChanged(int id);

	//! The local user's access to a feature has changed
	void featureAccessChanged(Feature feature, bool canUse);

	//! User lock/op/trust bits have changed
	void userBitsChanged(const AclState *acl);

	//! Feature access tiers have changed
	void featureTiersChanged(const rustpile::FeatureTiers&);

private:
	void emitFeatureChanges(int before, int now);

	struct Data;
	Data *d;
};

}

#endif
