-- Services
local RunService = game:GetService("RunService")
local Players = game:GetService("Players")
local UserInputService = game:GetService("UserInputService")
local TweenService = game:GetService("TweenService")

-- Constants
local PLAYER = Players.LocalPlayer
local LIVE_FOLDER = workspace.Live
local AUTO_FARM_DISTANCE = 5
local MOVE_COOLDOWN = 0.27
local MOVE_SWITCH_DELAY = 0.05
local CAMERA_OFFSET = CFrame.new(0, 2, 10)
local TOGGLE_KEY = Enum.KeyCode.KeypadZero

-- Combat Settings
local Combat = {
	KiMoves = {
		"Blaster Meteor",
		"Chain Destructo Disk"
	},
	MeleeMoves = {
		"Anger Rush",
		"Meteor Crash"
	},
	Trackables = {
		"KiBlast",
		"Blast"
	},
	PriorityTargets = {
		"Goku",
		"Vegeta",
		"Cabba",
		"Kale",
		"Caulifla",
		"Prum"
	},
	ExcludedNPCs = {
		"Jiren"
	}
}

-- State
local State = {
	running = false,
	currentTarget = nil,
	removedNPCs = {}
}

-- Utility Functions
local function isValidTarget(npc)
	return npc and 
		npc:FindFirstChildOfClass("Humanoid") and 
		npc:FindFirstChildOfClass("Humanoid").Health > 0 and
		npc:FindFirstChild("HumanoidRootPart")
end

local function getPlayerRoot()
	return PLAYER.Character and PLAYER.Character:FindFirstChild("HumanoidRootPart")
end

-- Target Management
local function checkPriorityTarget(targetName)
	if State.removedNPCs[targetName] then return end

	local npc = LIVE_FOLDER:FindFirstChild(targetName)
	if not npc then
		print(targetName .. " has been removed from the game.")
		State.removedNPCs[targetName] = true
		return
	end

	return isValidTarget(npc) and npc
end

local function getNextPriorityTarget()
	for _, targetName in ipairs(Combat.PriorityTargets) do
		local target = checkPriorityTarget(targetName)
		if target then return target end
	end
end

local function arePriorityTargetsDefeated()
	for _, targetName in ipairs(Combat.PriorityTargets) do
		if checkPriorityTarget(targetName) then return false end
	end
	return true
end

local function isEligibleNPC(npc)
	return not (table.find(Combat.PriorityTargets, npc.Name) or 
		table.find(Combat.ExcludedNPCs, npc.Name)) and
		npc:FindFirstChild("Damagers") and 
		isValidTarget(npc)
end

local function getClosestEligibleNPC()
	local playerRoot = getPlayerRoot()
	if not playerRoot then return end

	local closestNPC, closestDistance = nil, math.huge

	for _, npc in ipairs(LIVE_FOLDER:GetChildren()) do
		if isEligibleNPC(npc) then
			local npcRoot = npc:FindFirstChild("HumanoidRootPart")
			local distance = (npcRoot.Position - playerRoot.Position).Magnitude
			if distance < closestDistance then
				closestDistance = distance
				closestNPC = npc
			end
		end
	end

	return closestNPC
end

-- Combat Functions
local function updateKiPositions(targetPosition)
	for _, v in ipairs(workspace:GetChildren()) do
		if table.find(Combat.Trackables, v.Name) then
			v.Position = targetPosition
		end
	end

	local character = PLAYER.Character
	if character then
		for _, v in ipairs(character:GetChildren()) do
			if v:FindFirstChild("Ki") and v:FindFirstChild("Mesh") then
				v.Position = targetPosition
			end
		end
	end
end

local function teleportBehindTarget(target)
	local playerRoot = getPlayerRoot()
	local targetRoot = target and target:FindFirstChild("HumanoidRootPart")
	if not (playerRoot and targetRoot) then return end

	-- Update camera
	workspace.CurrentCamera.CFrame = CFrame.new(
		playerRoot.Position,
		Vector3.new(targetRoot.Position.X, playerRoot.Position.Y, targetRoot.Position.Z)
	) * CAMERA_OFFSET

	-- Calculate position behind target
	local targetCFrame = targetRoot.CFrame
	local behindPosition = targetCFrame * CFrame.new(0, 0, AUTO_FARM_DISTANCE)

	-- Teleport directly
	playerRoot.CFrame = behindPosition
end

local function useMoves(moveList)
	for _, move in ipairs(PLAYER.Backpack:GetChildren()) do
		if not State.running then break end
		if table.find(moveList, move.Name) then
			move.Parent = PLAYER.Character
			move:Activate()
			move:Deactivate()
			task.wait(MOVE_COOLDOWN)
			move.Parent = PLAYER.Backpack
			task.wait(MOVE_SWITCH_DELAY)
		end
	end
end

local function attackTarget(target, options)
	if not isValidTarget(target) then return false end

	print(string.format("Targeting %s: %s", 
		options.isPriority and "priority NPC" or "regular NPC",
		target.Name))

	-- Create update loop
	local updateConnection
	updateConnection = RunService.Heartbeat:Connect(function()
		if not (State.running and isValidTarget(target)) then
			updateConnection:Disconnect()
			return
		end

		local targetRoot = target:FindFirstChild("HumanoidRootPart")
		if targetRoot then
			if options.isPriority then
				teleportBehindTarget(target)
			else
				updateKiPositions(targetRoot.Position)
			end
		end
	end)

	-- Attack loop
	while State.running and isValidTarget(target) do
		useMoves(options.isPriority and Combat.MeleeMoves or Combat.KiMoves)
		task.wait()  -- Small yield to prevent overwhelming the game
	end

	updateConnection:Disconnect()
	print(target.Name .. " has been defeated!")
	return true
end

-- Main Loop
local function mainLoop()
	if not State.running then return end

	if not arePriorityTargetsDefeated() then
		if not isValidTarget(State.currentTarget) then
			State.currentTarget = getNextPriorityTarget()
			if State.currentTarget then
				attackTarget(State.currentTarget, {isPriority = true})
			end
		end
	else
		if not isValidTarget(State.currentTarget) then
			State.currentTarget = getClosestEligibleNPC()
			if State.currentTarget then
				attackTarget(State.currentTarget, {isPriority = false})
			else
				print("No eligible NPCs found.")
			end
		end
	end
end

-- Event Handlers
UserInputService.InputBegan:Connect(function(input, gameProcessed)
	if not gameProcessed and input.KeyCode == TOGGLE_KEY then
		State.running = not State.running
		if State.running then
			print("Script started.")
			State.removedNPCs = {}
		else
			print("Script stopped.")
			State.currentTarget = nil
		end
	end
end)

RunService.RenderStepped:Connect(mainLoop)
