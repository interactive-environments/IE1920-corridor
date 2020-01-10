const peakDistMax = 999999;

const n = 12;
const s = 1.5;

var triggered = Array(n).fill(0);
var offset = Array(n).fill(0);
var velocity = Array(n).fill(0);

var rot = Array(n).fill(0);

var prevMouseX = 0;
var prevMouseY = 0;

const defaultVelocity = 1;

var prevFrame = 0;

function setup() {
	createCanvas(100 * n, 400);
}

function velocityCurve(timeBetweenTriggers, entryPosition, prevVelocity) {
	// Target 5% in front.
	//const totalDist = 0.5 - entryPosition;
	const totalDist = 0.55 - entryPosition;
	const newVelocity = totalDist / (max(10, timeBetweenTriggers) / 1000) * 0.75;
	console.log(entryPosition + " " + totalDist + " " + timeBetweenTriggers + " " + newVelocity);
	if (prevVelocity < 0) {
		return newVelocity * 0.5;
	}
	return 0.4 * prevVelocity + 0.6 * newVelocity;
	//return totalDist / (timeBetweenTriggers / 1000) * 0.85;
	/*
	if (prevVelocity > 0) {
		const catchUpDist = 0.5 - entryPosition;
		const totalDist = catchUpDist + 1;
		const timeBetweenTriggers = 1 / prevVelocity;
		const newVelocity = totalDist / timeBetweenTriggers;
		console.log("R " + catchUpDist + " " + timeBetweenTriggers + " " + newVelocity);
		return (prevVelocity + newVelocity) / 2;
	} else {
		const catchUpDist = entryPosition - 0.5;
		const totalDist = catchUpDist + 1;
		const timeBetweenTriggers = 1 / -prevVelocity;
		const newVelocity = totalDist / timeBetweenTriggers;
		return -(-prevVelocity + newVelocity) / 2;
	}
	*/
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
			// Passed laser.
			if (triggered[i] == 0) {
				triggered[i] = now;
				if (i > 0 && triggered[i - 1] > 0) {
					// Going forward
					offset[i] = offset[i - 1] - 1;
					velocity[i] = velocityCurve(now - triggered[i - 1], offset[i], velocity[i - 1]);
					triggered[i - 1] = 0;
				} else if (i < n - 1 && triggered[i + 1] > 0) {
					// Going back
					offset[i] = offset[i + 1] + 1;
					velocity[i] = -velocityCurve(now - triggered[i + 1], -offset[i], -velocity[i + 1]);
					triggered[i + 1] = 0;
				} else {
					// Appeared out of nowhere
					offset[i] = 0;
					velocity[i] = 0;
				}
			}
		}
		// Timeout.
		if (now - triggered[i] > 2500) {
			triggered[i] = 0;
		}
	}
	
	prevMouseX = mouseX;
	prevMouseY = mouseY;
	
	// Draw lasers
	strokeWeight(1);
	for (var i = 0; i < n; i++) {
		if (triggered[i] == 0) {
			stroke(0);
		} else {
			stroke(255, 0, 0);
		}
		const laserX = (i + 0.05) * colw;
		line(laserX, 0, laserX, 400);
	}
	
	// Physics tick for wave peaks
	for (var i = 0; i < n; i++) {
		if (triggered[i] != 0) {
			if (velocity[i] > 0) {
				// Moving forward.
				offset[i] = min(offset[i] + velocity[i] * dt, 0.5);
			} else {
				// Moving backwards.
				offset[i] = max(offset[i] + velocity[i] * dt, -0.5);
			}
		}
	}
	
	// Draw panels
	stroke(127, 63, 0);
	strokeWeight(12);
	for (var i = 0; i < n; i++) {
		// Find distance to closest peak
		var peakDist = peakDistMax;
		for (var j = 0; j < n; j++) {
			if (triggered[j]) {
				const dist = abs(j + offset[j] - i);
				if (dist < peakDist) {
					peakDist = dist;
				}
			}
		}
		
		var targetRot = HALF_PI * 0.4 * pow((0.5 + 0.5 * sin(now / 1000 * PI * 0.5 - (i / n) * PI * 2)), 12);
		if (peakDist < peakDistMax) {
			targetRot = HALF_PI * peakNormalizedGaussian(peakDist, s);
		}
		rot[i] = rot[i] * 0.9 + targetRot * 0.1;
	
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
