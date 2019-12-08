const n = 12;
const s = 1.5;

var triggerTime = Array(n).fill(0);
var laserDiffTime = Array(n).fill(0);

var rot = Array(n).fill(0);
var prevMouseX = 0;
var prevMouseY = 0;

const defaultLaserDiffTime = 1000;
const constVelocityFactor = 0.33;
const velocityReduceFactor = 0.75;

var prevFrame = 0;

function setup() {
	createCanvas(100 * n, 400);
}

function draw() {
	// Frame-timing info
	const now = millis();
	const dt = (now - prevFrame) / 1000;
	prevFrame = now;
	
	background('#fff');
	
	colw = width / n;
	for (var i = 0; i < n; i++) {
		var laserX = (i + 0.05) * colw;
		if ((prevMouseX > laserX) != (mouseX > laserX)) {
			// Passed laser
			triggerTime[i] = now;
			laserDiffTime[i] = defaultLaserDiffTime;
			
			// Check for speed approximation
			if (i > 0 && triggerTime[i - 1] > 0) {
				laserDiffTime[i] = (now - triggerTime[i - 1]) * (1 - constVelocityFactor)
					+ laserDiffTime[i - 1] * constVelocityFactor;
				triggerTime[i - 1] = 0;
			}
			if (i < n - 1 && triggerTime[i + 1] > 0) {
				laserDiffTime[i] = (now - triggerTime[i + 1]) * (1 - constVelocityFactor)
					+ laserDiffTime[i + 1] * constVelocityFactor;
				triggerTime[i + 1] = 0;
			}
		}
	}
	
	prevMouseX = mouseX;
	prevMouseY = mouseY;
	
	// Draw lasers
	strokeWeight(1);
	for (var i = 0; i < n; i++) {
		if (triggerTime[i] == 0) {
			stroke(0);
		} else {
			stroke(255, 0, 0);
		}
		const laserX = (i + 0.05) * colw;
		line(laserX, 0, laserX, 400);
	}
	
	
	// Draw panels
	stroke(127, 63, 0);
	strokeWeight(12);
	for (var i = 0; i < n; i++) {
		var peakLaserDiffTime = defaultLaserDiffTime;
		var peakDist = 9999;
		for (var j = 0; j < n; j++) {
			if (triggerTime[j] != 0) {
				const dist = abs(j - i);
				if (dist < peakDist) {
					peakDist = dist;
					peakLaserDiffTime = laserDiffTime[j];
				}
			}
		}
		
		const targetRot = HALF_PI * unscaledGaussian(peakDist, s);
		if (abs(targetRot - rot[i]) < HALF_PI / 6) {
			rot[i] = rot[i] * 0.975 + targetRot * 0.025;
		} else {
			const laserDiffTimeS = min(peakLaserDiffTime, defaultLaserDiffTime) / 1000;
			// 1s laserDiffTime -> rotate by 90 deg every second
			// 0.5s laserDiffTime -> rotate by 180 deg every second
			const rotVelocity = HALF_PI / 2 / laserDiffTimeS;
			if (targetRot > rot[i]) {
				rot[i] = min(targetRot, rot[i] + rotVelocity * dt * velocityReduceFactor);
			} else if (targetRot < rot[i]) {
				rot[i] = max(targetRot, rot[i] - rotVelocity * dt * velocityReduceFactor);
			}
		}
	
		push();
		translate((i + 0.5) * colw, 40);
		rotate(-rot[i]);
		line(-0.35 * colw, 0, 0.35 * colw, 0);
		pop();
	}
}

function mouseClicked(){
	console.log(mouseX);
}

function unscaledGaussian(d, s) {
    return exp(-0.5 * pow(d / s, 2));
}

function peakNormalizedGaussian(d, s) {
    const v = unscaledGaussian(d, s);
    const peak = unscaledGaussian(0, s);
    return v / peak;
}
