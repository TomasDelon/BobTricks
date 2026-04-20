```mermaid
graph LR

    subgraph cfg["Config"]
        AppConfig
        ConfigIO
    end

    subgraph math_g["Math"]
        Vec2
        MathConstants
        Bezier
        StrokePath
    end

    subgraph chardata["Character · Data"]
        CMState
        FootState
        TrailPoint
        UpperBodyTypes
        BalanceState
        SupportState
        StepPlan
    end

    subgraph charlogic["Character · Logic"]
        CharacterState
        ArmController
        HeadController
        UpperBodyKinematics
    end

    subgraph loco["Locomotion"]
        BalanceComputer
        LegIK
        StandingController
        StepPlanner
        StepTriggerType
    end

    subgraph sim_g["Simulation"]
        InputFrame
        SimState
        SimulationCore
        GroundReference
        WorldConstants
        SimVerbosity
    end

    subgraph tele["Telemetry"]
        TelemetryRow
        TelemetryRecorder
    end

    subgraph terra["Terrain"]
        Terrain
    end

    subgraph rt["Runtime"]
        SimulationLoop
    end

    subgraph hl["Headless"]
        ScenarioDef
        ScenarioLibrary
        ScenarioRunner
    end

    subgraph rend["Render"]
        Camera2D
        DustParticle
        StrokeRenderer
        SceneRenderer
        CharacterRenderer
        DebugOverlayRenderer
    end

    subgraph app_g["App"]
        AudioSystem
        EffectsSystem
        InputController
        Application
    end

    subgraph dbg["Debug"]
        DebugUI
    end

    %% Math
    Vec2        --> MathConstants
    Bezier      --> Vec2
    StrokePath  --> Bezier

    %% Character data (leaves use Vec2 only — not shown)
    CMState          --> Vec2
    FootState        --> Vec2
    TrailPoint       --> Vec2
    UpperBodyTypes   --> Vec2
    StepPlan         --> Vec2

    %% Character logic
    CharacterState      --> Vec2
    CharacterState      --> CMState
    CharacterState      --> FootState
    CharacterState      --> AppConfig
    ArmController       --> AppConfig
    ArmController       --> CMState
    ArmController       --> CharacterState
    HeadController      --> AppConfig
    HeadController      --> CharacterState
    UpperBodyKinematics --> AppConfig
    UpperBodyKinematics --> ArmController
    UpperBodyKinematics --> HeadController
    UpperBodyKinematics --> UpperBodyTypes

    %% Locomotion
    BalanceComputer   --> BalanceState
    BalanceComputer   --> CMState
    BalanceComputer   --> SupportState
    BalanceComputer   --> AppConfig
    LegIK             --> Vec2
    StandingController --> SupportState
    StandingController --> CMState
    StandingController --> FootState
    StandingController --> AppConfig

    %% Terrain
    Terrain -.-> AppConfig
    Terrain     --> Vec2

    %% Simulation
    InputFrame      --> Vec2
    SimState        --> CMState
    SimState        --> CharacterState
    SimulationCore -.-> AppConfig
    SimulationCore  --> Terrain
    SimulationCore  --> SimState
    SimulationCore  --> InputFrame
    GroundReference --> AppConfig
    GroundReference --> CharacterState
    GroundReference --> CMState
    GroundReference --> Terrain

    %% Telemetry
    TelemetryRow      --> CharacterState
    TelemetryRecorder --> TelemetryRow
    TelemetryRecorder --> SimState

    %% Config
    ConfigIO --> AppConfig

    %% Headless
    ScenarioDef     --> SimState
    ScenarioDef     --> InputFrame
    ScenarioDef     --> TelemetryRecorder
    ScenarioLibrary --> AppConfig
    ScenarioLibrary --> ScenarioDef
    ScenarioRunner  --> AppConfig
    ScenarioRunner  --> ScenarioDef

    %% Render
    Camera2D            --> Vec2
    DustParticle        --> Vec2
    SceneRenderer       --> Camera2D
    SceneRenderer       --> DustParticle
    SceneRenderer       --> Terrain
    SceneRenderer       --> AppConfig
    CharacterRenderer   --> Camera2D
    CharacterRenderer   --> CMState
    CharacterRenderer   --> CharacterState
    CharacterRenderer   --> Terrain
    CharacterRenderer   --> AppConfig
    CharacterRenderer   --> StrokeRenderer
    DebugOverlayRenderer --> Camera2D
    DebugOverlayRenderer --> CMState
    DebugOverlayRenderer --> CharacterState
    DebugOverlayRenderer --> Terrain
    DebugOverlayRenderer --> AppConfig
    DebugOverlayRenderer --> TrailPoint

    %% App
    EffectsSystem   --> AppConfig
    EffectsSystem   --> SimState
    EffectsSystem   --> DustParticle
    InputController --> AppConfig
    InputController --> InputFrame
    InputController --> SimulationCore
    InputController --> Camera2D
    Application     --> AppConfig
    Application    -.-> SimulationCore
    Application     --> SimulationLoop
    Application     --> Camera2D
    Application     --> SceneRenderer
    Application     --> CharacterRenderer
    Application     --> DebugOverlayRenderer
    Application     --> DebugUI
    Application     --> AudioSystem
    Application     --> EffectsSystem
    Application     --> InputController
    Application     --> TrailPoint

    %% Debug UI
    DebugUI --> SimulationLoop
    DebugUI --> Terrain
    DebugUI --> Camera2D
    DebugUI --> CMState
    DebugUI --> CharacterState
    DebugUI --> AppConfig
```
